//
// Created by Laky64 on 22/04/2024.
//

#pragma once

#include <memory>

#include "file_parser.hpp"
#include "rtc/rtc.hpp"

namespace ntgcalls {
    class NTgCalls {
        std::unique_ptr<rtc::PeerConnection> connection;
        std::shared_ptr<Stream> stream;
        uint32_t audioSource = 0;
    public:
        std::string createCall();

        void connect(const std::string& params);
    };
} // ntgcalls
