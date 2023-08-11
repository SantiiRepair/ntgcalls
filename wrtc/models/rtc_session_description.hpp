//
// Created by Laky64 on 08/08/2023.
//

#pragma once

#include "rtc_session_description_init.hpp"
#include <future>

namespace wrtc {

    class Description {
    public:
        explicit Description(const RTCSessionDescriptionInit &rtcSessionDescriptionInit);

        static Description Wrap(webrtc::SessionDescriptionInterface *);

        explicit operator webrtc::SessionDescriptionInterface *();

        webrtc::SdpType getType();

        std::string getSdp();

    private:
        std::unique_ptr<webrtc::SessionDescriptionInterface> _description;
    };

} // namespace wrtc
