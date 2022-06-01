//
// cpu.h
// by Naomi Peori <naomi@peori.ca>
//

#pragma once

#include <stdlib.h>
#include "MOS6502.h"

class CPU : public MOS6502 {

public:

  CPU(int chrSize, uint8_t *chrData, int prgSize, uint8_t *prgData);

  // CPU
  uint8_t Load(const uint16_t address);
  void Store(const uint16_t address, const uint8_t value);

  // CHR
  uint8_t chrLoad(const uint16_t address);
  void chrStore(const uint16_t address, const uint8_t value);

  // PRG
  uint8_t prgLoad(const uint16_t address);
  void prgStore(const uint16_t address, const uint8_t value);

  // MMC1
  void mmc1Write(const uint16_t address, const uint8_t value);

  // Console
  void consoleWrite(const uint16_t address, const uint8_t value);

protected:

  // CPU
  uint8_t ram[0x800] = { 0 };

  // CHR
  int chrSize = 0;
  uint8_t *chrData = NULL;
  uint8_t chrRam[0x2000] = { 0 };

  // PRG
  int prgSize = 0;
  uint8_t *prgData = NULL;
  uint8_t prgRam[0x2000] = { 0 };

  // MMC1
  union { uint8_t d; struct { uint8_t chrBank:5; }; } chrRegister[ 2 ] = { { .d = 0x00 } };
  union { uint8_t d; struct { uint8_t mirroring:2, prgBankMode:2, chrBankMode:1; }; } controlRegister = { .d = 0x1C };
  union { uint8_t d; struct { uint8_t value:1, unused:6, reset:1; }; } loadRegister = { .d = 0x00 };
  union { uint8_t d; struct { uint8_t prgBank:4, prgRamDisabled:1; }; } prgRegister = { .d = 0x10 };
  union { uint8_t d; struct { uint8_t value:5, writes:3; }; } shiftRegister = { .d = 0x00 };

  // Console
  char consoleOutput[0x100] = { 0 };

};
