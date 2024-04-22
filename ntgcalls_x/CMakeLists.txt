file(GLOB_RECURSE MODULE_SRC *.cpp *.hpp)

pybind11_add_module(ntgcalls ${MODULE_SRC})
target_compile_definitions(ntgcalls PRIVATE VERSION_INFO=${PY_VERSION_INFO})
add_compile_definitions(PYTHON_ENABLED)

set_property(TARGET ntgcalls PROPERTY CXX_STANDARD 20 C_STANDARD 20)
target_link_libraries(ntgcalls PRIVATE nlohmann_json::nlohmann_json)
if(BOOST_ENABLED)
    target_link_libraries(ntgcalls PRIVATE Boost::filesystem)
endif ()
target_link_libraries(ntgcalls PRIVATE LibDataChannel::LibDataChannelStatic)

setup_platform_libs(ntgcalls)