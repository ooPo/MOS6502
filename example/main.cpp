//
// main.cpp
// by Naomi Peori <naomi@peori.ca>
//

#include <stdio.h>
#include <stdint.h>
#include "cpu.h"

struct HEADER
{
  uint8_t NES[4];   // 0x4E 0x45 0x53 0x1A
  uint8_t prgPages; // Number of 16k Pages
  uint8_t chrPages; // Number of  8k Pages
  uint8_t mapperLo;
  uint8_t mapperHi;
  uint8_t padding[8];
};

int main(int argc, char **argv) {

  HEADER header;

  int chrSize = 0;
  uint8_t *chrData = NULL;

  int prgSize = 0;
  uint8_t *prgData = NULL;

  if (argc < 2) {
    printf("USAGE: %s <filename.nes>\n", argv[0]);
  }

  FILE *romFile = fopen(argv[1], "r");
  if (romFile) {

    if (fread(&header, sizeof(HEADER), 1, romFile)) {

      prgSize = header.prgPages * 0x4000;
      prgData = (uint8_t *) malloc(prgSize);
      if (prgData) { fread(prgData, header.prgPages, 0x4000, romFile); }

      chrSize = header.chrPages * 0x2000;
      chrData = (uint8_t *) malloc(chrSize);
      if (chrData) { fread(prgData, header.chrPages, 0x2000, romFile); }

    }

    fclose(romFile);
  }

  if (prgSize && prgData) {
    CPU cpu(chrSize, chrData, prgSize, prgData);
    cpu.Reset();
    cpu.Run();
  }

  free(chrData);
  free(prgData);

  return 0;
}
