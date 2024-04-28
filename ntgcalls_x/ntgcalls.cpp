//
// Created by Laky64 on 22/04/2024.
//

#include "ntgcalls.hpp"

#include <random>
#include <string>

#include "models/call_payload.hpp"

namespace ntgcalls {

    std::string NTgCalls::createCall() {
        InitLogger(rtc::LogLevel::Verbose);
        rtc::Configuration config;
        config.iceTransportPolicy = rtc::TransportPolicy::All;
        config.enableIceUdpMux = true;
        connection = std::make_unique<rtc::PeerConnection>(config);
        connection->onStateChange([](const rtc::PeerConnection::State state) {
            std::cout << "State changed to " << state << std::endl;
        });

        // Add audio track
        // SSRC
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis;
        audioSource = dis(gen);

        auto audio = rtc::Description::Audio("audio-stream");
        audio.addOpusCodec(111);
        audio.addSSRC(audioSource, "audio-stream", "audio-stream", "audio-stream");
        const auto track = connection->addTrack(static_cast<rtc::Description::Media>(audio));

        auto rtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(
           audioSource,
           "audio-stream",
           111,
           rtc::OpusRtpPacketizer::DefaultClockRate
        );
        const auto packetizer = std::make_shared<rtc::OpusRtpPacketizer>(rtpConfig);
        const auto srReporter = std::make_shared<rtc::RtcpSrReporter>(rtpConfig);
        packetizer->addToChain(srReporter);
        const auto nackResponder = std::make_shared<rtc::RtcpNackResponder>();
        packetizer->addToChain(nackResponder);
        track->setMediaHandler(packetizer);

        stream = std::make_shared<Stream>(std::make_shared<FileParser>("/root/ntgcalls/tests/opus/", ".opus", 50, true));
        stream->onSample([track, rtpConfig, srReporter](Stream::StreamSourceType type, uint64_t sampleTime, rtc::binary sample) {
            std::string streamType = "audio";
            auto elapsedSeconds = static_cast<double>(sampleTime) / (1000 * 1000);
            uint32_t elapsedTimestamp = rtpConfig->secondsToTimestamp(elapsedSeconds);
            rtpConfig->timestamp = rtpConfig->startTimestamp + elapsedTimestamp;
            auto reportElapsedTimestamp = rtpConfig->timestamp - srReporter->lastReportedTimestamp();
            if (rtpConfig->timestampToSeconds(reportElapsedTimestamp) > 1) {
                srReporter->setNeedsToReport();
            }
            try {
                // send sample
                track->send(sample);
            } catch (const std::exception &e) {
                //std::cerr << "Unable to send "<< streamType << " packet: " << e.what() << std::endl;
            }
        });
        stream->start();
        // Set local description
        std::promise<rtc::Description> promise;
        connection->onLocalDescription([&](const rtc::Description &desc) {
            promise.set_value(desc);
        });
        connection->setLocalDescription(rtc::Description::Type::Offer);
        return std::string(CallPayload(promise.get_future().get()));
    }

    void NTgCalls::connect(const std::string& params) {
        auto data = json::parse(params)["transport"];
        Conference conference;
        try {
            conference = {
                {
                    data["ufrag"],
                    data["pwd"]
                },
                audioSource,
                {}
            };
            for (const auto& item : data["fingerprints"].items()) {
                conference.transport.fingerprints.push_back({
                    item.value()["hash"],
                    item.value()["fingerprint"],
                });
            }
            for (const auto& item : data["candidates"].items()) {
                conference.transport.candidates.push_back({
                    item.value()["generation"],
                    item.value()["component"],
                    item.value()["protocol"],
                    item.value()["port"],
                    item.value()["ip"],
                    item.value()["foundation"],
                    item.value()["id"],
                    item.value()["priority"],
                    item.value()["type"],
                    item.value()["network"]
                });
            }
        } catch (...) {
            throw std::runtime_error("Invalid transport");
        }
        const auto desc = rtc::Description(SdpBuilder::fromConference(conference), rtc::Description::Type::Answer);
        connection->setRemoteDescription(desc);
    }
} // ntgcalls