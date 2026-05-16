# MOS6502

A small, cycle-accurate C++ library for emulating a MOS 6502 CPU, designed for use in NES emulators and other 6502-based system emulators.

## Features

- All 56 official opcodes across all official addressing modes.
- Illegal (undocumented) opcode support, enabled at runtime via `enableIllegal`.
- Cycle-accurate: dummy reads and writes occur exactly as on real hardware.
- NMI (edge-triggered) and IRQ (level-triggered) interrupt support.
- BCD arithmetic support via `enableBCD`. (enabled by default; disable for NES/2A03)
- Unknown opcode callback for logging or custom behaviour.

## Project Layout

```
include/MOS6502/
  MOS6502.h             Core CPU class (abstract)

src/
  MOS6502.cpp           Opcode dispatch, addressing modes, and official operations
  MOS6502_illegal.cpp   Illegal opcode dispatch, addressing modes, and operations

examples/nes/
  cpu.h                 NES CPU class (derives from MOS6502)
  cpu.cpp               NES memory map, MMC1 mapper, and test ROM console output
  main.cpp              iNES ROM loader and entry point
```

## Building

Requires a C++17 compiler and CMake 3.21 or later.

```sh
cmake --preset default          # Configure (output in build/)
cmake --build --preset default  # Build
cmake --build --preset run      # Build and run the NES example against the test ROM
```

Without presets (CMake < 3.21):

```sh
cmake -B build
cmake --build build
cmake --build build --target run
```

## Using as a Library

### Via CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(MOS6502
    GIT_REPOSITORY https://github.com/ooPo/MOS6502.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(MOS6502)

target_link_libraries(my_target PRIVATE MOS6502)
```

### Via add_subdirectory

```cmake
add_subdirectory(path/to/MOS6502)
target_link_libraries(my_target PRIVATE MOS6502)
```

In both cases, `#include <MOS6502/MOS6502.h>` becomes available automatically.

## Usage

Derive from `MOS6502`, implement `Load()` and `Store()`, then call `Reset()` and `Run()`.

```cpp
class MySystem : public MOS6502 {
public:
    uint8_t Load(uint16_t address) override { ... }
    void    Store(uint16_t address, uint8_t value) override { ... }
};

MySystem sys;
sys.Reset();
sys.Run();    // returns when Halt() is called
```

### Runtime Flags

Set these in your derived class constructor before calling `Reset()`:

```cpp
enableBCD     = false; // disable decimal mode arithmetic (e.g. NES/2A03)
enableIllegal = true;  // enable illegal/undocumented opcodes (off by default)
```

### Interrupts

```cpp
sys.Signal(MOS6502::NMI, true);   // edge-triggered; CPU clears it on acknowledge
sys.Signal(MOS6502::IRQ, true);   // assert (level-triggered)
sys.Signal(MOS6502::IRQ, false);  // de-assert from inside Load/Store
```

`Signal()` is not thread-safe. It must be called from within `Load()` or `Store()` — i.e., on the same thread as `Run()`. Driving interrupts from a timer or audio callback requires external synchronisation.

### Stopping Execution

```cpp
void Store(uint16_t address, uint8_t value) override {
    ram[address] = value;
    if (address == 0x1234) { Halt(); }
}
```

### Unknown Opcodes

`OnUnknownOpcode` is called for any opcode not handled by the current configuration — unrecognised official opcodes always, and unrecognised illegal opcodes when `enableIllegal` is true:

```cpp
void OnUnknownOpcode(uint8_t opcode) override {
    std::fprintf(stderr, "Unknown opcode: %02X at PC=%04X\n", opcode, PC.w - 1);
    Halt();
}
```

### Illegal Opcodes

When `enableIllegal = true`, the following operations are supported:

| Mnemonic | Operation                    | Modes                                             |
|----------|------------------------------|---------------------------------------------------|
| DCP      | DEC, then CMP                | Abs, Abs,X, Abs,Y, ZP, ZP,X, (ZP,X), (ZP),Y       |
| ISC      | INC, then SBC                | Abs, Abs,X, Abs,Y, ZP, ZP,X, (ZP,X), (ZP),Y       |
| LAX      | Load A and X                 | Abs, Abs,Y, ZP, ZP,Y, (ZP,X), (ZP),Y              |
| RLA      | ROL, then AND                | Abs, Abs,X, Abs,Y, ZP, ZP,X, (ZP,X), (ZP),Y       |
| RRA      | ROR, then ADC                | Abs, Abs,X, Abs,Y, ZP, ZP,X, (ZP,X), (ZP),Y       |
| SAX      | Store A & X                  | Abs, ZP, ZP,Y, (ZP,X)                             |
| SLO      | ASL, then ORA                | Abs, Abs,X, Abs,Y, ZP, ZP,X, (ZP,X), (ZP),Y       |
| SRE      | LSR, then EOR                | Abs, Abs,X, Abs,Y, ZP, ZP,X, (ZP,X), (ZP),Y       |
| NOP      | No-op variants               | Implied ×6, Imm ×5, ZP ×3, ZP,X ×6, Abs, Abs,X ×6 |

All illegal RMW operations always pay the indexed page-cross cycle penalty.

`SHY`, `SHX`, and `TAS` are bus-timing-dependent on real hardware and their behaviour varies by board revision and capacitive load. The implementations follow the standard emulator approximation and will not exactly match a hardware logic analyser trace in all cases.

## Minimal Example

### cpu.h
```cpp
#pragma once
#include <cstdint>
#include <MOS6502/MOS6502.h>

class CPU : public MOS6502 {
public:
    explicit CPU(uint8_t *ram) : ram(ram) {}

    uint8_t Load(uint16_t address) override {
        return ram ? ram[address] : 0x00;
    }

    void Store(uint16_t address, uint8_t value) override {
        if (ram) { ram[address] = value; }
        if (address == 0x1234) { Halt(); }
    }

private:
    uint8_t *ram = nullptr;
};
```

### main.cpp
```cpp
#include <cstdio>
#include <cstdint>
#include "cpu.h"

int main() {
    uint8_t ram[65536] = {};

    FILE *f = std::fopen("rom.bin", "rb");
    if (f) { std::fread(ram, 1, sizeof(ram), f); std::fclose(f); }

    CPU cpu(ram);
    cpu.Reset();
    cpu.Run();
}
```

## Notes

- Cycle accuracy is implicit: every `Load()` and `Store()` call corresponds to one real CPU cycle. Per-cycle side effects (PPU tick, APU tick, mapper IRQ counters) can be driven from within those callbacks.
- There is no internal cycle counter exposed; the caller drives timing externally via the memory access callbacks.
- The NES example implements the MMC1 mapper (iNES mapper 1) and supports Blargg's `official_only.nes` test ROM.
- Inspired by the 6502 core in [higan](https://github.com/higan-emu/higan).

## License

BSD 2-Clause — see [LICENSE.txt](LICENSE.txt).
