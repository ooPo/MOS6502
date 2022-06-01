//
// cpu.cpp
// by Naomi Peori <naomi@peori.ca>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

CPU::CPU(int chrSize, uint8_t *chrData, int prgSize, uint8_t *prgData) {

  this->chrSize = chrSize;
  this->chrData = chrData;

  this->prgSize = prgSize;
  this->prgData = prgData;

  enableBCD = false;
}

uint8_t CPU::Load(const uint16_t address) {
  switch (address) {
    default: return 0x00;

    // RAM
    case 0x0000 ... 0x1FFF:
      return ram[address & 0x07FF];

    // CHR
    case 0x2000 ... 0x3FFF:
      return chrLoad(address);

    // PRG
    case 0x4020 ... 0xFFFF:
      return prgLoad(address);

  }
}

void CPU::Store(const uint16_t address, const uint8_t value) {
  switch(address) {

    // RAM
    case 0x0000 ... 0x1FFF:
      ram[address & 0x07FF] = value;
      break;

    // CHR
    case 0x2000 ... 0x3FFF:
      chrStore(address, value);
      break;

    // PRG
    case 0x4020 ... 0xFFFF:
      prgStore(address, value);
      consoleWrite(address, value);
      break;

  }
}

//
// CHR
//

uint8_t CPU::chrLoad(const uint16_t address) {
  switch (address) {
    default: return 0x00;

    case 0x2000 ... 0x3FFF:

      // Character RAM
      if (! chrSize) { return chrRam[address & 0x1FFF]; }

      // Character ROM
      if (! controlRegister.chrBankMode) {
        return chrData[((chrRegister[0].chrBank & 0x1E) << 12) | (address & 0x1FFF)];
      }

      return chrData[(chrRegister[(address >> 12) & 0x0001].chrBank << 12) | (address & 0x0FFF)];

  }
}

void CPU::chrStore(const uint16_t address, const uint8_t value) {
  switch (address) {

    // Character RAM
    case 0x2000 ... 0x3FFF:
      if (! chrSize) { chrRam[address & 0x1FFF] = value; }      
      break;

  }
}

//
// PRG
//

uint8_t CPU::prgLoad(const uint16_t address) {
  switch (address) {
    default: return 0x00;

    case 0x6000 ... 0x7FFF:
      return (prgRegister.prgRamDisabled) ? 0x00 : prgRam[address & 0x1FFF];

    case 0x8000 ... 0xFFFF:
      switch (controlRegister.prgBankMode) {
        default: return 0x00;

        case 0x00 ... 0x01:
          return prgData[((prgRegister.prgBank & 0x1E) << 14) | (address & 0x7FFF)];

        case 0x02:
          switch (address) {
            default: return 0x00;

            case 0x8000 ... 0xBFFF:
              return prgData[address & 0x3FFF];

            case 0xC000 ... 0xFFFF:
              return prgData[(prgRegister.prgBank << 14) | (address & 0x3FFF)];

          }

        case 0x03:
          switch (address) {
            default: return 0x00;

            case 0x8000 ... 0xBFFF:
              return prgData[(prgRegister.prgBank << 14) | (address & 0x3FFF)];

            case 0xC000 ... 0xFFFF:
              return prgData[(prgSize - 0x4000) | (address & 0x3FFF)];

          }

      }

  }
}

void CPU::prgStore(const uint16_t address, const uint8_t value) {
  switch (address) {

    case 0x6000 ... 0x7FFF:
      if (! prgRegister.prgRamDisabled) { prgRam[address & 0x1FFF] = value; }
      break;

    case 0x8000 ... 0xFFFF:
      loadRegister.d = value;

      // Reset the shift register.
      if (loadRegister.reset) {
        mmc1Write(0x8000, controlRegister.d | 0x0C);
        shiftRegister.d = 0x00;
      }

      // Write to the shift register.
      else {
        shiftRegister.value |= loadRegister.value << shiftRegister.writes++;

        // Write to the MMC1 register.
        if (shiftRegister.writes == 5) {
          mmc1Write(address, shiftRegister.value);
          shiftRegister.d = 0x00;
        }
      }

      break;
  }
}

//
// MMC1
//

void CPU::mmc1Write(uint16_t address, uint8_t value) {
  switch (address) {

    case 0x8000 ... 0x9FFF:
      controlRegister.d = value;
      break;

    case 0xA000 ... 0xBFFF:
      chrRegister[0].d = value;
      break;

    case 0xC000 ... 0xDFFF:
      chrRegister[1].d = value;
      break;

    case 0xE000 ... 0xFFFF:
      prgRegister.d = value;
      break;

  }
}

//
// Console
//

void CPU::consoleWrite(const uint16_t address, const uint8_t value) {
  switch ( address ) {

    case 0x6000:
      if (value != 0x80) { printf("Status: %02X\n", value); }
      if (value == 0x00) { printf("%s", consoleOutput); }
      break;

    case 0x6004 ... 0x6103:
      if (address == 0x6004) { printf("%s", consoleOutput); }
      consoleOutput[(address - 4) % sizeof(consoleOutput)] = value;
      break;

  }
}
