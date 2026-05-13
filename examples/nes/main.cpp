//
// main.cpp
// by Naomi Peori <naomi@peori.ca>
//

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include "cpu.h"

// iNES 1.0 ROM file header (16 bytes, no padding).
// Reference: https://www.nesdev.org/wiki/INES
struct iNESHeader {
  uint8_t magic[4];   // Must be { 0x4E, 0x45, 0x53, 0x1A } ("NES\x1A")
  uint8_t prgPages;   // Number of 16 KiB PRG-ROM banks
  uint8_t chrPages;   // Number of  8 KiB CHR-ROM banks (0 = CHR-RAM)
  uint8_t mapperLo;   // Flags 6: lower nibble of mapper number + mirroring/battery bits
  uint8_t mapperHi;   // Flags 7: upper nibble of mapper number + VS/PlayChoice bits
  uint8_t padding[8]; // Bytes 8-15: unused in iNES 1.0
};
static_assert(sizeof(iNESHeader) == 16, "iNESHeader must be exactly 16 bytes");

int main(int argc, char **argv) {

  if (argc < 2) {
    std::printf("USAGE: %s <filename.nes>\n", argv[0]);
    return 1;
  }

  // Open in binary mode — "r" (text mode) corrupts ROM data on Windows.
  FILE *romFile = std::fopen(argv[1], "rb");
  if (!romFile) {
    std::printf("ERROR: Could not open '%s'\n", argv[1]);
    return 1;
  }

  iNESHeader header;
  if (!std::fread(&header, sizeof(iNESHeader), 1, romFile)) {
    std::printf("ERROR: Could not read iNES header from '%s'\n", argv[1]);
    std::fclose(romFile);
    return 1;
  }

  // Validate the iNES magic number.
  const uint8_t magic[4] = { 0x4E, 0x45, 0x53, 0x1A };
  if (header.magic[0] != magic[0] || header.magic[1] != magic[1] ||
      header.magic[2] != magic[2] || header.magic[3] != magic[3]) {
    std::printf("ERROR: '%s' is not a valid iNES ROM file\n", argv[1]);
    std::fclose(romFile);
    return 1;
  }

  // Read PRG-ROM data.  fread(ptr, size, count, file) — size first, count second.
  const int prgSize = header.prgPages * 0x4000;
  std::vector<uint8_t> prgData(prgSize);
  if (!std::fread(prgData.data(), 0x4000, header.prgPages, romFile)) {
    std::printf("ERROR: Could not read PRG-ROM data\n");
    std::fclose(romFile);
    return 1;
  }

  // Read CHR-ROM data (may be zero pages, indicating CHR-RAM).
  const int chrSize = header.chrPages * 0x2000;
  std::vector<uint8_t> chrData(chrSize);
  if (chrSize && !std::fread(chrData.data(), 0x2000, header.chrPages, romFile)) {
    std::printf("ERROR: Could not read CHR-ROM data\n");
    std::fclose(romFile);
    return 1;
  }

  std::fclose(romFile);

  CPU cpu(chrSize, chrData.data(), prgSize, prgData.data());
  cpu.Reset();
  cpu.Run();

  return 0;
}
