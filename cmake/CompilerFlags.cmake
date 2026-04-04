if(MSVC)
    set(MSVC_COMMON_RELEASE "/O2 /Oi /Ot /GL /Gy /Ob3 /W3 /fp:precise /Qspectre-")
    set(MSVC_COMMON_DEBUG   "/Od /Zi /fp:strict")

    set(MSVC_CXX_EXTRA "/EHsc /permissive- /DNOMINMAX /DWIN32_LEAN_AND_MEAN")

    if(IS_XBOX AND IS_XDURANGO)
        set(MSVC_ARCH "/arch:SSE2")
    else()
        set(MSVC_ARCH "/arch:AVX2")
    endif()

    set(CMAKE_C_FLAGS_RELEASE   "${MSVC_COMMON_RELEASE} ${MSVC_ARCH}")
    set(CMAKE_CXX_FLAGS_RELEASE "${MSVC_COMMON_RELEASE} ${MSVC_ARCH} ${MSVC_CXX_EXTRA}")

    set(CMAKE_C_FLAGS_DEBUG     "${MSVC_COMMON_DEBUG}")
    set(CMAKE_CXX_FLAGS_DEBUG   "${MSVC_COMMON_DEBUG} ${MSVC_CXX_EXTRA}")

    set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "/LTCG /OPT:REF /OPT:ICF")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/LTCG /OPT:REF /OPT:ICF")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG      "/DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG   "/DEBUG")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(LTO_FLAG "-flto=thin")
    else()
        if(NIX)
            set(LTO_FLAG "")
        else()
            set(LTO_FLAG "-flto")
        endif()
    endif()

    set(COMMON_OPT  "-O3 ${LTO_FLAG} -funroll-loops -fomit-frame-pointer -Wno-missing-field-initializers")
    set(COMMON_FP   "-fno-fast-math")
    set(COMMON_WARN "-Wall -Wextra -pedantic")
    set(COMMON_CXX_LANG "-fexceptions -frtti")

    if(NIX)
        set(PLATFORM_OPT "${COMMON_OPT} -ffunction-sections -fdata-sections")
    else()
        set(PLATFORM_OPT "${COMMON_OPT}")
    endif()

    if(STATIC)
        set(STATIC_FLAG "-static")
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CLANG_USES_LLVM_RUNTIME)
            if(WIN32)
                set(STATIC_LIBS "")
            else()
                set(STATIC_LIBS "-static-libc++ -rtlib=compiler-rt -unwindlib=libunwind")
            endif()
        else()
            set(STATIC_LIBS "-static-libstdc++ -static-libgcc")
        endif()
    else()
        set(STATIC_FLAG "")
        set(STATIC_LIBS "")
    endif()

    if(NIX)
        set(CMAKE_C_FLAGS_RELEASE
            "${PLATFORM_OPT} ${COMMON_FP} ${COMMON_WARN}"
        )
        set(CMAKE_CXX_FLAGS_RELEASE
            "${PLATFORM_OPT} ${COMMON_FP} ${COMMON_CXX_LANG} ${COMMON_WARN}"
        )
    else()
        if(NATIVE)
            set(CMAKE_C_FLAGS_RELEASE
                "${PLATFORM_OPT} ${COMMON_FP} ${COMMON_WARN} -march=native"
            )
            set(CMAKE_CXX_FLAGS_RELEASE
                "${PLATFORM_OPT} ${COMMON_FP} ${COMMON_CXX_LANG} ${COMMON_WARN} -march=native"
            )
        else()
            set(CMAKE_C_FLAGS_RELEASE
                "${PLATFORM_OPT} ${COMMON_FP} ${COMMON_WARN} -march=x86-64-v3"
            )
            set(CMAKE_CXX_FLAGS_RELEASE
                "${PLATFORM_OPT} ${COMMON_FP} ${COMMON_CXX_LANG} ${COMMON_WARN} -march=x86-64-v3"
            )
        endif()
    endif()

    if(APPLE)
        set(EXE_LINKER_FLAGS    "-Wl,-dead_strip ${STATIC_FLAG} ${STATIC_LIBS}")
        set(SHARED_LINKER_FLAGS "-Wl,-dead_strip ${STATIC_LIBS}")
    elseif(WIN32)
        set(EXE_LINKER_FLAGS    "${STATIC_FLAG} ${STATIC_LIBS}")
        set(SHARED_LINKER_FLAGS "${STATIC_LIBS}")
    else()
        set(EXE_LINKER_FLAGS    "-Wl,--gc-sections ${STATIC_FLAG} ${STATIC_LIBS}")
        set(SHARED_LINKER_FLAGS "-Wl,--gc-sections ${STATIC_LIBS}")
    endif()

    set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${EXE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${SHARED_LINKER_FLAGS}")

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND APPLE)
        set(DEBUG_INFO "-glldb")
    else()
        set(DEBUG_INFO "-g")
    endif()

    set(CMAKE_C_FLAGS_DEBUG
        "-O0 ${DEBUG_INFO} -fno-omit-frame-pointer -fno-fast-math"
    )
    set(CMAKE_CXX_FLAGS_DEBUG
        "-O0 ${DEBUG_INFO} -fno-omit-frame-pointer -fexceptions -frtti -fno-fast-math"
    )

else()
    message(WARNING "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}. No custom flags applied.")
endif()

if(WIN32 AND STATIC OR IS_XBOX)
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "$<$<CONFIG:Debug>:MultiThreadedDebug>$<$<NOT:$<CONFIG:Debug>>:MultiThreaded>"
    )
elseif(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "$<$<CONFIG:Debug>:MultiThreadedDebugDLL>$<$<NOT:$<CONFIG:Debug>>:MultiThreadedDLL>"
    )
endif()
