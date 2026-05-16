cmake_minimum_required(VERSION 3.15)
project(MOS6502 VERSION 1.0.0 LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(_mos6502_top_level ON)
else()
    set(_mos6502_top_level OFF)
endif()
option(MOS6502_BUILD_EXAMPLES "Build the MOS6502 NES example" ${_mos6502_top_level})

add_library(MOS6502 STATIC
    src/MOS6502.cpp
    src/MOS6502_illegal.cpp
)

target_include_directories(MOS6502 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(MOS6502 PUBLIC cxx_std_17)

target_compile_options(MOS6502 PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-gnu-anonymous-struct>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-gnu-case-range>
)

if(MOS6502_BUILD_EXAMPLES)
    add_subdirectory(examples/nes)
endif()
