//
// cpu.h
// by Naomi Peori <naomi@peori.ca>
//

#pragma once

#include <cstdint>
#include "MOS6502.h"

class CPU : public MOS6502 {

public:

  CPU(int chrSize, uint8_t *chrData, int prgSize, uint8_t *prgData);

  // MOS6502 interface
  uint8_t Load(uint16_t address) override;
  void    Store(uint16_t address, uint8_t value) override;

protected:

  // CPU RAM: $0000–$07FF mirrored through $1FFF.
  uint8_t ram[0x0800] = {};

  //
  // CHR (PPU bus, accessed via CPU-side PPU register reads/writes at $2000–$3FFF)
  //

  int      chrSize = 0;
  uint8_t *chrData = nullptr;

  // CHR-RAM: used when the cartridge has no CHR-ROM (chrSize == 0).
  uint8_t chrRam[0x2000] = {};

  uint8_t chrLoad(uint16_t address);
  void    chrStore(uint16_t address, uint8_t value);

  //
  // PRG (CPU bus, $6000–$FFFF)
  //

  int      prgSize = 0;
  uint8_t *prgData = nullptr;

  // PRG-RAM (battery-backed save RAM), $6000–$7FFF.
  uint8_t prgRam[0x2000] = {};

  uint8_t prgLoad(uint16_t address);
  void    prgStore(uint16_t address, uint8_t value);

  //
  // MMC1 mapper registers
  //
  // All MMC1 registers are 5-bit values written serially through shiftRegister.
  // Reference: https://www.nesdev.org/wiki/MMC1
  //

  // $8000–$9FFF: Control register.
  //   mirroring   [1:0] — nametable mirroring mode
  //   prgBankMode [3:2] — PRG bank switching mode (0/1 = 32 KiB, 2 = fix low, 3 = fix high)
  //   chrBankMode [4]   — CHR bank switching mode (0 = 8 KiB, 1 = two 4 KiB)
  union {
    uint8_t d;
    struct { uint8_t mirroring:2, prgBankMode:2, chrBankMode:1; };
  } controlRegister = { .d = 0x1C }; // power-on: prgBankMode=3 (last bank fixed)

  // $A000–$BFFF and $C000–$DFFE: CHR bank registers 0 and 1.
  //   chrBank [4:0] — selects a 4 KiB or 8 KiB CHR bank depending on chrBankMode
  union {
    uint8_t d;
    struct { uint8_t chrBank:5; };
  } chrRegister[2] = {};

  // $E000–$FFFF: PRG bank register.
  //   prgBank       [3:0] — selects a 16 KiB PRG bank
  //   prgRamDisabled  [4] — disables PRG-RAM when set
  union {
    uint8_t d;
    struct { uint8_t prgBank:4, prgRamDisabled:1; };
  } prgRegister = { .d = 0x10 }; // power-on: PRG-RAM enabled, bank 0

  // Serial load register: bits are shifted in LSB-first over 5 writes.
  //   value [4:0] — accumulated shift data
  //   writes [7:5] — number of bits written so far (cleared with d=0x00)
  union {
    uint8_t d;
    struct { uint8_t value:5, writes:3; };
  } shiftRegister = {};

  // CPU-side load register: used to inspect the reset bit before shifting.
  //   value [0]  — the data bit to shift in
  //   reset [7]  — when set, resets the shift register immediately
  union {
    uint8_t d;
    struct { uint8_t value:1, unused:6, reset:1; };
  } loadRegister = {};

  void mmc1Write(uint16_t address, uint8_t value);

  //
  // nestest / blargg test ROM console output
  //
  // Test ROMs signal status at $6000 and write text to $6004–$6103.
  // The buffer is exactly 256 bytes to match the $6004–$6103 address range.
  // Reference: https://www.nesdev.org/wiki/Emulator_tests
  //

  char consoleOutput[0x100] = {};

  void consoleWrite(uint16_t address, uint8_t value);

};
