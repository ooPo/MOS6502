# MOS6502
This is a small and simple library for emulating a MOS6502 cpu.

## Usage
- Implement the Load() and Store() member functions.
- Initialize with Reset().
- Start execution with Run().
- Execution will end when Halt() is received.

## Notes
* Uses techniques inspired by the 6502 core in [higan-emu](https://github.com/higan-emu/higan "higan-emu").
* Only official opcodes are supported.
  * Successfully passes Blargg's "official_only.nes" testing rom.
  * BCD support is unimplemented currently.
* While cycle-accurate, there is no internal cycle counter.
  * 6502 cycles implicitly occur on every memory access.
  * Per-cycle operations can be handled by Load() and Store().

## Example Usage

### cpu.h
```
#pragma once

#include <stdlib.h>
#include "MOS6502.h"

class CPU : public MOS6502 {

	public:
		CPU(uint8_t *ram);
		uint8_t Load(const uint16_t address);
		void Store(const uint16_t address, const uint8_t value);

	protected:
		uint8_t *ram = NULL;

};
```

### cpu.cpp
```
#include <stdlib.h>
#include "cpu.h"

CPU::CPU(uint8_t *ram) {
	this->ram = ram;
}

uint8_t CPU::Load(const uint16_t address) {
	if (ram) { return ram[address]; }
	return 0x00;
}

void CPU::Store(const uint16_t address, const uint8_t value) {
	if (ram) { ram[address] = value; }

	// Stop execution on writes to 0x1234.
	if (address == 0x1234) { Halt(); }
}
```

### main.cpp
```
#include <stdio.h>
#include <stdint.h>

#include "cpu.h"

int main(void) {
	int romSize = 0;
	uint8_t romData[ 65536 ] = { 0 };

	FILE *romFile = fopen("rom.bin", "r");
	if (romFile) {
		romSize = fread(romData, 1, 65536, romFile);
		fclose(romFile);
	}

	if (romSize) {
		CPU cpu(romData);
		cpu.Reset();
		cpu.Run();
	}

	return 0;
}
```
