cmake_policy(SET CMP0144 NEW)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

message(STATUS "Checking for Boost...")

if(DEFINED ENV{BOOST_ROOT})
    set(PHASOR_BOOST ON)
endif()

if(PHASOR_BOOST)
    set(Boost_ROOT "$ENV{BOOST_ROOT}" CACHE PATH "Boost root")
    find_package(Boost REQUIRED COMPONENTS)
else()
    message(STATUS "Set envvar BOOST_ROOT to enable Boost library")
endif()