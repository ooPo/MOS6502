cmake_minimum_required(VERSION 3.15)
project(MOS6502 VERSION 1.0.0 LANGUAGES CXX)

# When used via add_subdirectory, skip the example by default
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(_mos6502_top_level ON)
else()
    set(_mos6502_top_level OFF)
endif()
option(MOS6502_BUILD_EXAMPLES "Build the MOS6502 NES example" ${_mos6502_top_level})

add_subdirectory(MOS6502)

if(MOS6502_BUILD_EXAMPLES)
    add_subdirectory(examples/nes)
endif()
