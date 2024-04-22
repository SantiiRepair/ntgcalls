//
// Created by Laky64 on 22/04/2024.
//

#include "file_parser.hpp"

#include <fstream>

namespace ntgcalls {
    FileParser::FileParser(std::string directory, std::string extension, uint32_t samplesPerSecond, bool loop) {
        this->directory = std::move(directory);
        this->extension = std::move(extension);
        this->loop = loop;
        this->sampleDuration_us = 1000 * 1000 / samplesPerSecond;
    }

    FileParser::~FileParser() {
        stop();
    }

    void FileParser::start() {
        sampleTime_us = std::numeric_limits<uint64_t>::max() - sampleDuration_us + 1;
        loadNextSample();
    }

    void FileParser::stop() {
        sample = {};
        sampleTime_us = 0;
        counter = -1;
    }

    void FileParser::loadNextSample() {
        std::string frame_id = std::to_string(++counter);

        std::string url = directory + "/sample-" + frame_id + extension;
        std::ifstream source(url, std::ios_base::binary);
        if (!source) {
            if (loop && counter > 0) {
                loopTimestampOffset = sampleTime_us;
                counter = -1;
                loadNextSample();
                return;
            }
            sample = {};
            return;
        }

        const std::vector contents((std::istreambuf_iterator(source)), std::istreambuf_iterator<char>());
        auto *b = reinterpret_cast<const std::byte*>(contents.data());
        sample.assign(b, b + contents.size());
        sampleTime_us += sampleDuration_us;
    }

    rtc::binary FileParser::getSample() {
        return sample;
    }

    uint64_t FileParser::getSampleTime_us() {
        return sampleTime_us;
    }

    uint64_t FileParser::getSampleDuration_us() {
        return sampleDuration_us;
    }

    Stream::Stream(std::shared_ptr<StreamSource> audio): audio(audio) { }

    Stream::~Stream() {
        stop();
    }

    std::pair<std::shared_ptr<StreamSource>, Stream::StreamSourceType> Stream::unsafePrepareForSample() {
        std::shared_ptr<StreamSource> ss;
        StreamSourceType sst;
        uint64_t nextTime;
        ss = audio;
        sst = StreamSourceType::Audio;
        nextTime = audio->getSampleTime_us();

        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

        auto elapsed = currentTime.count() - startTime;
        if (nextTime > elapsed) {
            auto waitTime = nextTime - elapsed;
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(waitTime));
            mutex.lock();
        }
        return {ss, sst};
    }

    void Stream::sendSample() {
        thread = std::thread([this] {
            while (_isRunning) {
                auto [ss, sst] = unsafePrepareForSample();
                if (!isRunning) {
                    break;
                }
                sampleHandler(sst, ss->getSampleTime_us(), ss->getSample());
                ss->loadNextSample();
            }
        });
        thread.detach();
    }

    void Stream::onSample(std::function<void (StreamSourceType, uint64_t, rtc::binary)> handler) {
        sampleHandler = handler;
    }

    void Stream::start() {
        std::lock_guard lock(mutex);
        if (isRunning) {
            return;
        }
        _isRunning = true;
        startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        audio->start();
        sendSample();
    }

    void Stream::stop() {
        std::lock_guard lock(mutex);
        if (!isRunning) {
            return;
        }
        _isRunning = false;
        audio->stop();
    };
} // ntgcalls