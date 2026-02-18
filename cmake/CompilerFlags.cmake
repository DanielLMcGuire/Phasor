if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE
        "/O2 /Oi /Ot /GL /Gy /MT /fp:precise /arch:AVX2 /Qspectre-"
    )
    set(CMAKE_CXX_FLAGS_RELEASE
        "/O2 /Oi /Ot /GL /Gy /MT /fp:precise /arch:AVX2 /EHsc /permissive- /DNOMINMAX"
    )
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "/LTCG /OPT:REF /OPT:ICF"
    )
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
        "/LTCG /OPT:REF /OPT:ICF"
    )
    set(CMAKE_C_FLAGS_DEBUG "/Od /Zi /fp:strict")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /fp:strict /EHsc /DNOMINMAX")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG")
else()
    set(COMMON_OPT
        "-O3 -flto -funroll-loops -fomit-frame-pointer -Wno-missing-field-initializers"
    )
    set(COMMON_FP
        "-ffast-math"
    )
    set(COMMON_WARN
        "-Wall -Wextra -pedantic"
    )
    set(COMMON_CXX_LANG
        "-fexceptions -frtti"
    )
    
    set(CMAKE_C_FLAGS_RELEASE
        "${COMMON_OPT} ${COMMON_FP} -march=native"
    )

    set(CMAKE_CXX_FLAGS_RELEASE
        "${COMMON_OPT} ${COMMON_FP} ${COMMON_CXX_LANG} ${COMMON_WARN} -march=native"
    )
    
    if(APPLE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-dead_strip")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-Wl,-dead_strip")
    elseif(WIN32)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "")
    else()
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")
    endif()
    
    set(CMAKE_C_FLAGS_DEBUG
        "-O0 -g -fno-omit-frame-pointer"
    )
    set(CMAKE_CXX_FLAGS_DEBUG
        "-O0 -g -fno-omit-frame-pointer -fexceptions -frtti"
    )
endif()