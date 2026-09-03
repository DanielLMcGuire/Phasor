set(ZLIB_COMPAT ON)

set(SKIP_INSTALL_ALL ON) 
set(ZLIB_ENABLE_TESTS OFF)

set(CMAKE_INSTALL_LIBDIR "lib") 
set(CMAKE_INSTALL_INCLUDEDIR "include")

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG        2.2.1
)
FetchContent_MakeAvailable(zlib)

if(TARGET zlibstatic AND NOT TARGET ZLIB::ZLIBSTATIC)
    add_library(ZLIB::ZLIBSTATIC ALIAS zlibstatic)
else()
    set(PHASOR_ZLIB OFF)
endif()