if(MSVC)
    if(IS_XBOX)
        if(IS_XDURANGO)
            set(CMAKE_C_FLAGS_RELEASE
                "/O2 /Oi /Ot /GL /Ob3 /Gy /MT /fp:precise /arch:SSE2 /Qspectre-"
            )
            set(CMAKE_CXX_FLAGS_RELEASE
                "/O2 /Oi /Ot /GL /Gy /Ob3 /MT /fp:precise /arch:SSE2 /EHsc /permissive- /DNOMINMAX /DWIN32_LEAN_AND_MEAN"
            )
        else()
            set(CMAKE_C_FLAGS_RELEASE
                "/O2 /Oi /Ot /GL /Gy /Ob3 /MT /fp:precise /arch:AVX2 /Qspectre-"
            )
            set(CMAKE_CXX_FLAGS_RELEASE
                "/O2 /Oi /Ot /GL /Gy /Ob3 /MT /fp:precise /arch:AVX2 /EHsc /permissive- /DNOMINMAX /DWIN32_LEAN_AND_MEAN"
            )
        endif()
    else()
        set(CMAKE_C_FLAGS_RELEASE
            "/O2 /Oi /Ot /GL /Gy /Ob3 /fp:precise /arch:AVX2 /Qspectre-"
        )
        set(CMAKE_CXX_FLAGS_RELEASE
            "/O2 /Oi /Ot /GL /Gy /Ob3 /fp:precise /arch:AVX2 /EHsc /permissive- /DNOMINMAX /DWIN32_LEAN_AND_MEAN"
        )
    endif()
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "/LTCG /OPT:REF /OPT:ICF"
    )
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
        "/LTCG /OPT:REF /OPT:ICF"
    )
    set(CMAKE_C_FLAGS_DEBUG "/Od /MTd /Zi /fp:strict")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /MTd /Zi /fp:strict /EHsc /DNOMINMAX /DWIN32_LEAN_AND_MEAN")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG")
else()
    if(NIX)
        set(COMMON_OPT
            "-O3 -funroll-loops -fomit-frame-pointer -Wno-missing-field-initializers"
        )
    else()
        set(COMMON_OPT
            "-O3 -flto -funroll-loops -fomit-frame-pointer -Wno-missing-field-initializers"
        )
    endif()  
    set(COMMON_OPT_NIX
        "-O3 -funroll-loops -fomit-frame-pointer -Wno-missing-field-initializers"
    )
    set(COMMON_FP
        "-fno-fast-math"
    )
    set(COMMON_WARN
        "-Wall -Wextra -pedantic -Wno-nan-infinity-disabled"
    )
    set(COMMON_CXX_LANG
        "-fexceptions -frtti"
    )
    if(NIX)
        set(CMAKE_C_FLAGS_RELEASE
            "${COMMON_OPT} ${COMMON_FP}"
        )
    else()
        set(CMAKE_C_FLAGS_RELEASE
            "${COMMON_OPT} ${COMMON_FP} -march=native"
        )
    endif()
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