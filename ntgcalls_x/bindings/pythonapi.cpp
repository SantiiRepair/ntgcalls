//
// Created by Laky64 on 12/08/2023.
//
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../ntgcalls.hpp"
#include "../models/media_description.hpp"

namespace py = pybind11;

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

PYBIND11_MODULE(ntgcalls, m) {
    py::class_<ntgcalls::NTgCalls> wrapper(m, "NTgCalls");
    wrapper.def(py::init<>());
    wrapper.def("create_call", &ntgcalls::NTgCalls::createCall);
    wrapper.def("connect", &ntgcalls::NTgCalls::connect, py::arg("params"));

    py::enum_<ntgcalls::BaseMediaDescription::InputMode>(m, "InputMode")
            .value("FILE", ntgcalls::BaseMediaDescription::InputMode::File)
            .value("SHELL", ntgcalls::BaseMediaDescription::InputMode::Shell)
            .value("FFMPEG", ntgcalls::BaseMediaDescription::InputMode::FFmpeg)
            .value("NO_LATENCY", ntgcalls::BaseMediaDescription::InputMode::NoLatency)
            .export_values()
            .def("__and__",[](const ntgcalls::BaseMediaDescription::InputMode& lhs, const ntgcalls::BaseMediaDescription::InputMode& rhs) {
                return static_cast<ntgcalls::BaseMediaDescription::InputMode>(lhs & rhs);
            })
            .def("__and__",[](const ntgcalls::BaseMediaDescription::InputMode& lhs, const int rhs) {
                return static_cast<ntgcalls::BaseMediaDescription::InputMode>(lhs & rhs);
            })
            .def("__or__",[](const ntgcalls::BaseMediaDescription::InputMode& lhs, const ntgcalls::BaseMediaDescription::InputMode& rhs) {
                return static_cast<ntgcalls::BaseMediaDescription::InputMode>(lhs | rhs);
            })
            .def("__or__",[](const ntgcalls::BaseMediaDescription::InputMode& lhs, const int rhs) {
                return static_cast<ntgcalls::BaseMediaDescription::InputMode>(lhs | rhs);
            });

    py::class_<ntgcalls::BaseMediaDescription> mediaWrapper(m, "BaseMediaDescription");
    mediaWrapper.def_readwrite("input", &ntgcalls::BaseMediaDescription::input);

    py::class_<ntgcalls::AudioDescription> audioWrapper(m, "AudioDescription", mediaWrapper);
    audioWrapper.def(
            py::init<ntgcalls::BaseMediaDescription::InputMode, uint32_t, uint8_t, uint8_t, std::string>(),
            py::arg("input_mode"),
            py::arg("sample_rate"),
            py::arg("bits_per_sample"),
            py::arg("channel_count"),
            py::arg("input")
    );
    audioWrapper.def_readwrite("sampleRate", &ntgcalls::AudioDescription::sampleRate);
    audioWrapper.def_readwrite("bitsPerSample", &ntgcalls::AudioDescription::bitsPerSample);
    audioWrapper.def_readwrite("channelCount", &ntgcalls::AudioDescription::channelCount);

    py::class_<ntgcalls::VideoDescription> videoWrapper(m, "VideoDescription", mediaWrapper);
    videoWrapper.def(
            py::init<ntgcalls::BaseMediaDescription::InputMode, uint16_t, uint16_t, uint8_t, std::string>(),
            py::arg("input_mode"),
            py::arg("width"),
            py::arg("height"),
            py::arg("fps"),
            py::arg("input")
    );
    videoWrapper.def_readwrite("width", &ntgcalls::VideoDescription::width);
    videoWrapper.def_readwrite("height", &ntgcalls::VideoDescription::height);
    videoWrapper.def_readwrite("fps", &ntgcalls::VideoDescription::fps);

    py::class_<ntgcalls::MediaDescription> mediaDescWrapper(m, "MediaDescription");
    mediaDescWrapper.def(
            py::init<std::optional<ntgcalls::AudioDescription>, std::optional<ntgcalls::VideoDescription>>(),
            py::arg_v("audio", std::nullopt, "None"),
            py::arg_v("video", std::nullopt, "None")
    );
    mediaDescWrapper.def_readwrite("audio", &ntgcalls::MediaDescription::audio);
    mediaDescWrapper.def_readwrite("video", &ntgcalls::MediaDescription::video);

    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
}