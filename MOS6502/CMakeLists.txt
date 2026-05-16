add_library(MOS6502 STATIC
    MOS6502.cpp
    MOS6502_illegal.cpp
)

target_include_directories(MOS6502 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(MOS6502 PUBLIC cxx_std_17)

# Suppress warnings for GNU extensions used internally (anonymous structs, case ranges)
target_compile_options(MOS6502 PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-gnu-anonymous-struct>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-gnu-case-range>
)
