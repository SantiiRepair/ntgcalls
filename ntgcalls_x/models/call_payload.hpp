//
// Created by Laky64 on 21/08/2023.
//

#pragma once

#include "rtc/description.hpp"
#include <nlohmann/json.hpp>


namespace ntgcalls {
    using nlohmann::json;
    struct Fingerprint {
        std::string hash;
        std::string fingerprint;
    };

    struct Candidate {
        std::string generation;
        std::string component;
        std::string protocol;
        std::string port;
        std::string ip;
        std::string foundation;
        std::string id;
        std::string priority;
        std::string type;
        std::string network;
    };

    struct Transport {
        std::string ufrag;
        std::string pwd;
        std::vector<Fingerprint> fingerprints;
        std::vector<Candidate> candidates;
    };

    struct Conference {
        Transport transport;
        uint32_t ssrc;
        std::vector<uint32_t> source_groups;
    };

    class SdpBuilder {
    public:
        std::vector<std::string> lines;
        std::vector<std::string> newLine;

        [[nodiscard]] std::string join() const;

        [[nodiscard]] std::string finalize() const;
        void add(const std::string& line);
        void push(const std::string& word);
        void addJoined(const std::string& separator = "");
        void addCandidate(const Candidate& c);
        void addHeader();
        void addTransport(const Transport& transport);
        void addSsrcEntry(const Transport& transport);
        void addConference(const Conference& conference);
        static std::string fromConference(const Conference& conference);
    };
    class CallPayload {
    public:
        std::string ufrag;
        std::string pwd;
        std::string hash;
        std::string fingerprint;
        uint32_t audioSource;
        std::vector<uint32_t> sourceGroups;

        explicit CallPayload(const rtc::Description &desc);

        explicit operator std::string() const;
    };

} // ntgcalls

