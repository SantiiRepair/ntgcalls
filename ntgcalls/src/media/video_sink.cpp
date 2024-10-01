//
// Created by Laky64 on 27/09/24.
//

#include <cmath>
#include <ntgcalls/media/video_sink.hpp>
#include <rtc_base/logging.h>

namespace ntgcalls {

    std::chrono::nanoseconds VideoSink::frameTime() {
        if (!description) return std::chrono::nanoseconds(0);
        return std::chrono::microseconds(static_cast<uint64_t>(1000.0 * 1000.0 / static_cast<double_t>(description->fps))); // ms
    }

    int64_t VideoSink::frameSize() {
        if (!description) return 0;
        return std::llround(static_cast<float>(description->width * description->height) * 1.5f);
    }

    bool VideoSink::setConfig(const std::optional<VideoDescription>& desc) {
        clear();
        const bool changed = description != desc;
        description = desc;
        RTC_LOG(LS_INFO) << "VideoSink configured with " << desc->width << "x" << desc->height << "@" << desc->fps << "fps";
        return changed;
    }

} // ntgcalls