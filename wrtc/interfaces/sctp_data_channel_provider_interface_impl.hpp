//
// Created by Laky64 on 29/03/2024.
//

#pragma once
#include <iostream>
#include <rtc_base/weak_ptr.h>
#include <api/data_channel_interface.h>
#include <pc/sctp_data_channel.h>
#include <media/sctp/sctp_transport_factory.h>

#include "wrtc/utils/binary.hpp"
#include "wrtc/utils/syncronized_callback.hpp"

namespace wrtc {

    class SctpDataChannelProviderInterfaceImpl final : public sigslot::has_slots<>, public webrtc::SctpDataChannelControllerInterface, public webrtc::DataChannelObserver, public webrtc::DataChannelSink {
        rtc::WeakPtrFactory<SctpDataChannelProviderInterfaceImpl> weakFactory;
        std::unique_ptr<cricket::SctpTransportFactory> sctpTransportFactory;
        std::unique_ptr<cricket::SctpTransportInternal> sctpTransport;
        rtc::scoped_refptr<webrtc::SctpDataChannel> dataChannel;
        rtc::Thread* networkThread;
        bool isOpen = false;
        bool isSctpTransportStarted = false;

        synchronized_callback<bool> onStateChangedCallback;

    public:
        SctpDataChannelProviderInterfaceImpl(
            rtc::PacketTransportInternal* transportChannel,
            bool isOutgoing,
            rtc::Thread* networkThread,
            rtc::Thread* signalingThread
        );

        ~SctpDataChannelProviderInterfaceImpl() override;

        bool IsOkToCallOnTheNetworkThread() override;

        void OnDataReceived(int channel_id, webrtc::DataMessageType type, const rtc::CopyOnWriteBuffer& buffer) override;

        void OnReadyToSend() override;

        void OnStateChange() override;

        void OnMessage(const webrtc::DataBuffer& buffer) override;

        webrtc::RTCError SendData(webrtc::StreamId sid, const webrtc::SendDataParams& params, const rtc::CopyOnWriteBuffer& payload) override;

        void AddSctpDataStream(webrtc::StreamId sid) override;

        void RemoveSctpDataStream(webrtc::StreamId sid) override;

        void updateIsConnected(bool isConnected);

        void sendDataChannelMessage(const bytes::binary& data) const;

        // Unused
        void OnChannelClosing(int channel_id) override
        {
            std::cout << "OnChannelClosing" << std::endl;
        }
        void OnChannelClosed(int channel_id) override
        {
            std::cout << "OnChannelClosed" << std::endl;
        }
        void OnChannelStateChanged(webrtc::SctpDataChannel* data_channel, webrtc::DataChannelInterface::DataState state) override
        {
            std::cout << "OnChannelStateChanged" << std::endl;
        }
        void OnTransportClosed(webrtc::RTCError error) override
        {
            std::cout << "OnTransportClosed" << std::endl;
        };

        void onStateChanged(const std::function<void(bool)>& callback);

    };

} // wrtc