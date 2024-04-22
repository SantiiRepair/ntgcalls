//
// Created by Laky64 on 21/08/2023.
//

#include "call_payload.hpp"

#include <random>

namespace ntgcalls {
    CallPayload::CallPayload(const rtc::Description &desc) {
        ufrag = desc.iceUfrag().value();
        pwd = desc.icePwd().value();
        hash = rtc::CertificateFingerprint::AlgorithmIdentifier(desc.fingerprint()->algorithm);
        fingerprint = desc.fingerprint()->value;

        for (size_t i = 0; i < desc.mediaCount(); i++) {
            std::visit(rtc::overloaded{[&](auto) {},
            [&](const rtc::Description::Media *media) {
                if (media->type() == "audio") {
                    for (const auto &ssrc : media->getSSRCs()) {
                        audioSource = ssrc;
                        break;
                    }
                }
            }}, desc.media(i));
        }
        //for (auto &ssrc : source_groups) {
        //    this->sourceGroups.push_back(static_cast<wrtc::TgSSRC>(ssrc));
        //}
    }

    CallPayload::operator std::string() const {
        json jsonRes = {
            {"ufrag", ufrag},
            {"pwd", pwd},
            {"fingerprints", {
                    {
                        {"hash", hash},
                        {"setup", "active"},
                        {"fingerprint", fingerprint}
                    }
            }},
        };
        jsonRes["ssrc"] = audioSource;
        if (!sourceGroups.empty()){
            jsonRes["ssrc-groups"] = {
                {
                    {"semantics", "FID"},
                    {"sources", sourceGroups}
                }
            };
        }
        return to_string(jsonRes);
    }

    std::string SdpBuilder::join() const {
        std::string joinedSdp;
        for (const auto& line : lines) {
            joinedSdp += line + "\r\n";
        }
        return joinedSdp;
    }

    std::string SdpBuilder::finalize() const {
        return join();
    }

    void SdpBuilder::add(const std::string& line) {
        lines.push_back(line);
    }

    void SdpBuilder::push(const std::string& word) {
        newLine.push_back(word);
    }

    void SdpBuilder::addJoined(const std::string& separator) {
        std::string joinedLine;
        for (size_t i = 0; i < newLine.size(); ++i) {
            joinedLine += newLine[i];
            if (i != newLine.size() - 1) {
                joinedLine += separator;
            }
        }
        add(joinedLine);
        newLine.clear();
    }

    void SdpBuilder::addCandidate(const Candidate& c) {
        push("a=candidate:");
        push(c.foundation + " " + c.component + " " + c.protocol + " " + c.priority + " " + c.ip + " " + c.port + " typ " + c.type);
        push(" generation " + c.generation);
        addJoined();
    }

    void SdpBuilder::addHeader() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int64_t> dis;

        add("v=0");
        add("o=- " + std::to_string(dis(gen)) + " 2 IN IP4 0.0.0.0");
        add("s=-");
        add("t=0 0");
        add("a=group:BUNDLE 0 1");
        add("a=ice-lite");
    }

    void SdpBuilder::addTransport(const Transport& transport) {
        add("a=ice-ufrag:" + transport.ufrag);
        add("a=ice-pwd:" + transport.pwd);

        for (const auto& [hash, fingerprint] : transport.fingerprints) {
            push("a=fingerprint:");
            push(hash);
            push(" ");
            push(fingerprint);
            addJoined();
            add("a=setup:passive");
        }

        for (const auto& candidate : transport.candidates) {
            addCandidate(candidate);
        }
    }

    void SdpBuilder::addSsrcEntry(const Transport& transport) {
        //AUDIO CODECS
        add("m=audio 1 RTP/SAVPF 111 126");
        add("c=IN IP4 0.0.0.0");
        add("a=mid:0");

        addTransport(transport);

        // OPUS CODEC
        add("a=rtpmap:111 opus/48000/2");
        add("a=rtpmap:126 telephone-event/8000");
        add("a=fmtp:111 minptime=10; useinbandfec=1; usedtx=1");
        add("a=rtcp:1 IN IP4 0.0.0.0");
        add("a=rtcp-mux");
        add("a=rtcp-fb:111 transport-cc");
        add("a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level");
        add("a=recvonly");
        //END AUDIO CODECS

        //VIDEO CODECS
        add("m=video 1 RTP/SAVPF 100 101 102 103");
        add("c=IN IP4 0.0.0.0");
        add("a=mid:1");
        addTransport(transport);

        //VP8 CODEC
        add("a=rtpmap:100 VP8/90000/1");
        add("a=fmtp:100 x-google-start-bitrate=800");
        add("a=rtcp-fb:100 goog-remb");
        add("a=rtcp-fb:100 transport-cc");
        add("a=rtcp-fb:100 ccm fir");
        add("a=rtcp-fb:100 nack");
        add("a=rtcp-fb:100 nack pli");
        add("a=rtpmap:101 rtx/90000");
        add("a=fmtp:101 apt=100");

        //VP9 CODEC
        add("a=rtpmap:102 VP9/90000/1");
        add("a=rtcp-fb:102 goog-remb");
        add("a=rtcp-fb:102 transport-cc");
        add("a=rtcp-fb:102 ccm fir");
        add("a=rtcp-fb:102 nack");
        add("a=rtcp-fb:102 nack pli");
        add("a=rtpmap:103 rtx/90000");
        add("a=fmtp:103 apt=102");

        add("a=recvonly");
        add("a=rtcp:1 IN IP4 0.0.0.0");
        add("a=rtcp-mux");
        //END VIDEO CODECS
    }

    void SdpBuilder::addConference(const Conference& conference) {
        addHeader();
        addSsrcEntry(conference.transport);
    }

    std::string SdpBuilder::fromConference(const Conference& conference) {
        SdpBuilder sdp;
        sdp.addConference(conference);
        return sdp.finalize();
    }
} // ntgcalls