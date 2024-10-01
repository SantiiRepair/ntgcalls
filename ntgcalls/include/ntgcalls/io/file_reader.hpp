//
// Created by Laky64 on 04/08/2023.
//

#pragma once

#include <fstream>
#include <string>

#include <ntgcalls/exceptions.hpp>
#include <ntgcalls/io/threaded_reader.hpp>

namespace ntgcalls {
    class FileReader final: public ThreadedReader {
        std::ifstream source;

        bytes::unique_binary read(int64_t size) override;

    public:
        explicit FileReader(const std::string& path, BaseSink *sink);

        ~FileReader() override;
    };
}
