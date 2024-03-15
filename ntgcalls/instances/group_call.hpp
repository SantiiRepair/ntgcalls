//
// Created by Laky64 on 15/03/2024.
//
#pragma once
#include "call_interface.hpp"
#include "wrtc/enums.hpp"

namespace ntgcalls {

    class GroupCall final : public CallInterface {
        wrtc::SSRC audioSource = 0;
        std::vector<wrtc::SSRC> sourceGroups = {};
    public:
        ~GroupCall() override;

        std::string init(const MediaDescription& config);

        void connect(const std::string& jsonData);

        Type type() const override;
    };

} // ntgcalls
