//
// Created by Laky64 on 17/09/24.
//

#include <ntgcalls/devices/media_device.hpp>

#include <utility>
#include <ntgcalls/exceptions.hpp>
#include <ntgcalls/devices/alsa_device_module.hpp>
#include <ntgcalls/devices/input_device.hpp>

namespace ntgcalls {
    std::unique_ptr<BaseReader> MediaDevice::CreateInput(const BaseMediaDescription& desc, const int64_t bufferSize) {
        if (auto* audio = dynamic_cast<const AudioDescription*>(&desc)) {
            return CreateAudioInput(audio, bufferSize);
        }
        throw MediaDeviceError("Unsupported media type");
    }

    std::unique_ptr<BaseReader> MediaDevice::CreateAudioInput(const AudioDescription* desc, int64_t bufferSize) {
        std::unique_ptr<BaseDeviceModule> adm;
#ifdef IS_LINUX
        if (AlsaDeviceModule::isSupported()) {
            RTC_LOG(LS_INFO) << "Using ALSA module for input";
            adm = std::make_unique<AlsaDeviceModule>(desc, true);
        }
#endif
        if (adm) {
            return std::make_unique<InputDevice>(desc, std::move(adm), bufferSize);
        }
        throw MediaDeviceError("Unsupported platform for audio device");
    }
} // ntgcalls