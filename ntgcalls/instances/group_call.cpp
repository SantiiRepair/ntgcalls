//
// Created by Laky64 on 15/03/2024.
//

#include "group_call.hpp"

#include "ntgcalls/exceptions.hpp"
#include "ntgcalls/models/call_payload.hpp"
#include "wrtc/utils/sync.hpp"

namespace ntgcalls {
    GroupCall::~GroupCall() {
        sourceGroups.clear();
    }

    std::string GroupCall::init(const MediaDescription& config) {
        if (connection) {
            throw ConnectionError("Connection already made");
        }
        connection = std::make_shared<wrtc::PeerConnection>();
        stream->addTracks(connection);
        const std::optional offer = connection->createOffer(false, false);
        connection->setLocalDescription(offer);
        const auto payload = CallPayload(offer.value());
        stream->setAVStream(config, true);
        audioSource = payload.audioSource;
        for (const auto &ssrc : payload.sourceGroups) {
            sourceGroups.push_back(ssrc);
        }
        return payload;
    }

    void GroupCall::connect(const std::string& jsonData) {
        if (!connection) {
            throw ConnectionError("Connection not initialized");
        }
        auto data = json::parse(jsonData);
        if (!data["rtmp"].is_null()) {
            throw RTMPNeeded("Needed rtmp connection");
        }
        if (data["transport"].is_null()) {
            throw InvalidParams("Transport not found");
        }
        data = data["transport"];
        wrtc::Conference conference;
        try {
            conference = {
                {
                    data["ufrag"].get<std::string>(),
                    data["pwd"].get<std::string>()
                },
                audioSource,
                sourceGroups
            };
            for (const auto& item : data["fingerprints"].items()) {
                conference.transport.fingerprints.push_back({
                    item.value()["hash"],
                    item.value()["fingerprint"],
                });
            }
            for (const auto& item : data["candidates"].items()) {
                conference.transport.candidates.push_back({
                    item.value()["generation"].get<std::string>(),
                    item.value()["component"].get<std::string>(),
                    item.value()["protocol"].get<std::string>(),
                    item.value()["port"].get<std::string>(),
                    item.value()["ip"].get<std::string>(),
                    item.value()["foundation"].get<std::string>(),
                    item.value()["id"].get<std::string>(),
                    item.value()["priority"].get<std::string>(),
                    item.value()["type"].get<std::string>(),
                    item.value()["network"].get<std::string>()
                });
            }
        } catch (...) {
            throw InvalidParams("Invalid transport");
        }

        const auto remoteDescription = wrtc::Description(
            wrtc::Description::Type::Answer,
            wrtc::SdpBuilder::fromConference(conference)
        );
        connection->setRemoteDescription(remoteDescription);

        wrtc::Sync<void> waitConnection;
        connection->onIceStateChange([&](const wrtc::IceState state) {
            switch (state) {
            case wrtc::IceState::Connected:
                    if (!this->connected) waitConnection.onSuccess();
                    break;
                case wrtc::IceState::Disconnected:
                case wrtc::IceState::Failed:
                case wrtc::IceState::Closed:
                    if (!this->connected) {
                        waitConnection.onFailed(std::make_exception_ptr(TelegramServerError("Telegram Server is having some internal problems")));
                    } else {
                        connection->onIceStateChange(nullptr);
                        (void) this->onCloseConnection();
                    }
                    break;
                default:
                    break;
            }
        });
        waitConnection.wait();
        this->connected = true;
        stream->start();
    }

    CallInterface::Type GroupCall::type() const {
        return Type::Group;
    }
} // ntgcalls