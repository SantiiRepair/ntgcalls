//
// Created by Laky64 on 22/04/2024.
//

#pragma once

#include <string>
#include <vector>
#include <thread>
#include "rtc/common.hpp"

namespace ntgcalls {
    class StreamSource {
    public:
        virtual ~StreamSource() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void loadNextSample() = 0;

        virtual uint64_t getSampleTime_us() = 0;
        virtual uint64_t getSampleDuration_us() = 0;
        virtual rtc::binary getSample() = 0;
    };

    class FileParser: public StreamSource {
        std::string directory;
        std::string extension;
        uint64_t sampleDuration_us;
        uint64_t sampleTime_us = 0;
        uint32_t counter = -1;
        bool loop;
        uint64_t loopTimestampOffset = 0;
    protected:
        rtc::binary sample = {};
    public:
        FileParser(std::string directory, std::string extension, uint32_t samplesPerSecond, bool loop);
        ~FileParser() override;
        void start() override;
        void stop() override;
        void loadNextSample() override;

        rtc::binary getSample() override;
        uint64_t getSampleTime_us() override;
        uint64_t getSampleDuration_us() override;
    };

    class Stream: public std::enable_shared_from_this<Stream> {
        uint64_t startTime = 0;
        std::mutex mutex;
        bool _isRunning = false;
        std::thread thread;


    public:
        const std::shared_ptr<StreamSource> audio;

        Stream(std::shared_ptr<StreamSource> audio);
        ~Stream();

        enum class StreamSourceType {
            Audio,
            Video
        };

    private:
        rtc::synchronized_callback<StreamSourceType, uint64_t, rtc::binary> sampleHandler;

        std::pair<std::shared_ptr<StreamSource>, StreamSourceType> unsafePrepareForSample();

        void sendSample();

    public:
        void onSample(std::function<void (StreamSourceType, uint64_t, rtc::binary)> handler);
        void start();
        void stop();
        const bool & isRunning = _isRunning;
    };

} // ntgcalls
