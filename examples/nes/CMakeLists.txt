add_executable(mos6502_example
    main.cpp
    cpu.cpp
)

target_link_libraries(mos6502_example PRIVATE MOS6502)
