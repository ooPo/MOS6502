//
// cpu.cpp
// by Naomi Peori <naomi@peori.ca>
//

#include <cstdio>
#include <cstring>
#include "cpu.h"

CPU::CPU(int chrSize, uint8_t *chrData, int prgSize, uint8_t *prgData) {
  this->chrSize = chrSize;
  this->chrData = chrData;
  this->prgSize = prgSize;
  this->prgData = prgData;
  enableBCD = false;
}

// ---------------------------------------------------------------------------
// CPU bus: Load / Store
//
// NES CPU memory map:
//   $0000–$1FFF  Internal RAM (2 KiB, mirrored ×4)
//   $2000–$3FFF  PPU registers (8 registers, mirrored)
//   $4000–$401F  APU and I/O registers (not emulated here)
//   $4020–$5FFF  Cartridge expansion area
//   $6000–$7FFF  PRG-RAM (battery-backed)
//   $8000–$FFFF  PRG-ROM (mapper-controlled)
// ---------------------------------------------------------------------------

uint8_t CPU::Load(uint16_t address) {
  switch (address) {
    default: return 0x00;

    case 0x0000 ... 0x1FFF: return ram[address & 0x07FF];
    case 0x2000 ... 0x3FFF: return chrLoad(address);
    case 0x4020 ... 0xFFFF: return prgLoad(address);
  }
}

void CPU::Store(uint16_t address, uint8_t value) {
  switch (address) {
    default: break;

    case 0x0000 ... 0x1FFF: ram[address & 0x07FF] = value;    break;
    case 0x2000 ... 0x3FFF: chrStore(address, value);         break;
    case 0x4020 ... 0x5FFF: /* expansion — not implemented */ break;

    case 0x6000 ... 0xFFFF:
      prgStore(address, value);
      // Test ROM console output lives in the PRG-RAM window ($6000–$61FF).
      if (address <= 0x6103) { consoleWrite(address, value); }
      break;
  }
}

// ---------------------------------------------------------------------------
// CHR: PPU register access from the CPU bus ($2000–$3FFF)
//
// From the CPU's perspective these are PPU control/data registers.
// The internal CHR-ROM/RAM is on the PPU bus ($0000–$1FFF on the PPU side);
// chrLoad/chrStore here handle the CPU-visible PPU register window.
// ---------------------------------------------------------------------------

uint8_t CPU::chrLoad(uint16_t address) {
  // CHR bank mode selects either one 8 KiB bank (mode 0) or two 4 KiB banks (mode 1).
  // The address is masked to $0000–$1FFF (the PPU CHR window) before banking.
  if (!chrSize) {
    return chrRam[address & 0x1FFF];
  }

  if (!controlRegister.chrBankMode) {
    // 8 KiB mode: chrRegister[0] selects the bank; low bit is ignored.
    return chrData[((chrRegister[0].chrBank & 0x1E) << 12) | (address & 0x1FFF)];
  }

  // 4 KiB mode: chrRegister[0] for $0000–$0FFF, chrRegister[1] for $1000–$1FFF.
  return chrData[(chrRegister[(address >> 12) & 0x01].chrBank << 12) | (address & 0x0FFF)];
}

void CPU::chrStore(uint16_t address, uint8_t value) {
  // Writes only affect CHR-RAM cartridges; CHR-ROM is read-only.
  if (!chrSize) {
    chrRam[address & 0x1FFF] = value;
  }
}

// ---------------------------------------------------------------------------
// PRG: cartridge ROM/RAM access ($4020–$FFFF)
// ---------------------------------------------------------------------------

uint8_t CPU::prgLoad(uint16_t address) {
  switch (address) {
    default: return 0x00;

    case 0x6000 ... 0x7FFF:
      return prgRegister.prgRamDisabled ? 0x00 : prgRam[address & 0x1FFF];

    case 0x8000 ... 0xFFFF:
      switch (controlRegister.prgBankMode) {
        default: return 0x00;

        case 0x00 ... 0x01:
          // 32 KiB mode: prgRegister selects a 32 KiB block; low bit ignored.
          return prgData[((prgRegister.prgBank & 0x1E) << 14) | (address & 0x7FFF)];

        case 0x02:
          // Fix-low mode: $8000–$BFFF is fixed to bank 0; $C000–$FFFF is switchable.
          if (address <= 0xBFFF) {
            return prgData[address & 0x3FFF];
          }
          return prgData[(prgRegister.prgBank << 14) | (address & 0x3FFF)];

        case 0x03:
          // Fix-high mode: $8000–$BFFF is switchable; $C000–$FFFF is fixed to last bank.
          if (address <= 0xBFFF) {
            return prgData[(prgRegister.prgBank << 14) | (address & 0x3FFF)];
          }
          return prgData[(prgSize - 0x4000) | (address & 0x3FFF)];
      }
  }
}

void CPU::prgStore(uint16_t address, uint8_t value) {
  switch (address) {

    case 0x6000 ... 0x7FFF:
      if (!prgRegister.prgRamDisabled) {
        prgRam[address & 0x1FFF] = value;
      }
      break;

    case 0x8000 ... 0xFFFF:
      loadRegister.d = value;

      if (loadRegister.reset) {
        // A write with bit 7 set resets the shift register and restores
        // prgBankMode to 3 (fix-high) in the control register.
        mmc1Write(0x8000, controlRegister.d | 0x0C);
        shiftRegister.d = 0x00;
      } else {
        // Shift the data bit in LSB-first.
        shiftRegister.value |= loadRegister.value << shiftRegister.writes++;

        if (shiftRegister.writes == 5) {
          mmc1Write(address, shiftRegister.value);
          shiftRegister.d = 0x00;
        }
      }
      break;
  }
}

// ---------------------------------------------------------------------------
// MMC1 internal register write (called after 5 serial bits are accumulated)
// ---------------------------------------------------------------------------

void CPU::mmc1Write(uint16_t address, uint8_t value) {
  switch (address) {
    case 0x8000 ... 0x9FFF:  controlRegister.d  = value;  break;
    case 0xA000 ... 0xBFFF:  chrRegister[0].d   = value;  break;
    case 0xC000 ... 0xDFFF:  chrRegister[1].d   = value;  break;
    case 0xE000 ... 0xFFFF:  prgRegister.d      = value;  break;
  }
}

// ---------------------------------------------------------------------------
// Test ROM console output
//
// Follows the blargg/nestest convention:
//   $6000        — status byte (0x00 = all tests passed, 0x80 = running)
//   $6001–$6003  — unused
//   $6004–$6103  — null-terminated result string (256 bytes)
//
// The buffer is printed each time a new string starts (address wraps to
// $6004) and when the status byte signals completion (value == 0x00).
// ---------------------------------------------------------------------------

void CPU::consoleWrite(uint16_t address, uint8_t value) {
  switch (address) {

    case 0x6000:
      if (value != 0x80) {
        std::printf("Status: %02X\n", value);
      }
      if (value == 0x00) {
        std::printf("%s", consoleOutput);
      }
      break;

    case 0x6004 ... 0x6103: {
      const int index = address - 0x6004;
      // Flush the previous message when the write pointer wraps back to the start.
      if (index == 0) {
        std::printf("%s", consoleOutput);
      }
      consoleOutput[index] = static_cast<char>(value);
      break;
    }

  }
}
