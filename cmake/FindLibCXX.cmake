if(LINUX)
    set(LIBCXX_INCLUDE ${deps_loc}/libcxx)
    set(LIBCXXABI_INCLUDE ${deps_loc}/libcxxabi)

    GitClone(
        URL https://chromium.googlesource.com/external/github.com/llvm/llvm-project/libcxx.git
        COMMIT ${LIBCXX_COMMIT}
        DIRECTORY ${LIBCXX_INCLUDE}
    )
    GitFile(
        URL https://chromium.googlesource.com/chromium/src/buildtools.git/+/refs/heads/main/third_party/libc++/__config_site
        DIRECTORY ${LIBCXX_INCLUDE}/include/__config_site
    )
    GitClone(
        URL https://chromium.googlesource.com/external/github.com/llvm/llvm-project/libcxxabi.git
        COMMIT ${LIBCXX_ABI_COMMIT}
        DIRECTORY ${LIBCXXABI_INCLUDE}
    )
    add_compile_options(
        "$<$<COMPILE_LANGUAGE:CXX>:-nostdinc++>"
        "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:LIBCXX_INCLUDE_DIR>>:-isystem${LIBCXX_INCLUDE}/include>"
        "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:LIBCXXABI_INCLUDE_DIR>>:-isystem${LIBCXXABI_INCLUDE}/include>"
    )
endif ()