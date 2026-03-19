add_compile_definitions(CMAKE)

if(IS_TRACING)
    add_compile_definitions(TRACING)
endif()

if(IS_XSCARLET)
    add_compile_definitions(PLATFORM_OVERRIDE)
    add_compile_definitions(PLATFORM_MODERN_GAME)
endif()

if(IS_XDURANGO)
    add_compile_definitions(PLATFORM_OVERRIDE)
    add_compile_definitions(PLATFORM_LEGACY_GAME)
endif()