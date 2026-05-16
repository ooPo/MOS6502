add_executable(mos6502_example
    main.cpp
    cpu.cpp
)

target_link_libraries(mos6502_example PRIVATE MOS6502)

add_custom_target(run
    COMMAND mos6502_example "${CMAKE_CURRENT_SOURCE_DIR}/official_only.nes"
    DEPENDS mos6502_example
)
