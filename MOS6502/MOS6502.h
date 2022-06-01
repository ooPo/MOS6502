//
// MOS6502.h
// by Naomi Peori (naomi@peori.ca)
//

#pragma once
#include <stdlib.h>

class MOS6502
{

public:

  using BYTE = uint8_t;

  using WORD = union {
    uint16_t w;

#ifdef __LITTLE_ENDIAN__
    struct { uint8_t l, h; };
#else
    struct { uint8_t h, l; };
#endif
  };

  enum INTERRUPT { NMI = 0, IRQ = 1, COUNT = 2 };

  void Run(void);
  
  inline void Halt(void) { this->running = false; }
  inline void Signal(const INTERRUPT interrupt, const bool value) { this->signals[interrupt] = value; }
  inline void Reset(void) { Interrupt(0xFFFC); }

  virtual uint8_t Load(const uint16_t address) = 0;
  virtual void Store(const uint16_t address, const uint8_t value) = 0;

protected:

  bool enableBCD = false;

  bool running = false;
  bool signals[INTERRUPT::COUNT] = { false };

  WORD PC = { .w = 0x0000 };
  WORD AB = { .w = 0x0000 };
  WORD TB = { .w = 0x0000 };

  BYTE A = 0x00;
  BYTE X = 0x00;
  BYTE Y = 0x00;
  BYTE S = 0xFF;

  union {
    BYTE value;
    struct { uint8_t C:1, Z:1, I:1, D:1, B:1, T:1, V:1, N:1; };
  }
  P = { .value = 0x34 };

protected:

  //
  // Helpers
  //

  inline void Branch(bool test) {
    int8_t offset = Fetch();
    if (test) {
      IdleOnPageCrossed(PC, offset);
      Idle();
      PC.w += offset;
    }
  }

  inline uint8_t Fetch(void) {
    return Load(PC.w++);
  }

  inline uint8_t Flags(uint8_t value) {
    P.N = (value & 0x80) ? 1 : 0;
    P.Z = (value & 0xFF) ? 0 : 1;
    return value;
  }

  inline void Idle(void) {
    Load(PC.w);
  }

  inline WORD IdleOnPageAlways(WORD address, BYTE index) {
    TB.w = AB.w + index;
    Load(AB.h | TB.l); // dummy read
    return TB;
  }

  inline WORD IdleOnPageCrossed(WORD address, BYTE index) {
    TB.w = AB.w + index;
    if (AB.l > TB.l) { Load(AB.h | TB.l); } // dummy read
    return TB;
  }

  inline void Interrupt(uint16_t vector) {
    Push(PC.h);
    Push(PC.l);
    Push(P.value | 0x20);
    PC.l = Load(vector);
    PC.h = Load(vector + 1);
    P.I = 1;
  }

  inline uint8_t Pull(void) {
    return Load(0x0100 | ++S);
  }

  inline void Push(const uint8_t value) {
    Store(0x0100 | S--, value);
  }

  //
  // Addressing Modes
  //

  using OPERATION = void (MOS6502::*)(const BYTE input, BYTE &output);

  void Absolute_Modify(OPERATION operation);
  void Absolute_Modify(OPERATION operation, BYTE index);
  void Absolute_Read(OPERATION operation, BYTE &output);
  void Absolute_Read(OPERATION operation, BYTE &output, BYTE index);
  void Absolute_Write(BYTE &input);
  void Absolute_Write(BYTE &input, BYTE index);
  void Immediate_Read(OPERATION operation, BYTE &output);
  void IndexedIndirect_Read(OPERATION operation, BYTE &output, BYTE index);
  void IndexedIndirect_Write(BYTE &input, BYTE index);
  void IndirectIndexed_Read(OPERATION operation, BYTE &output, BYTE index);
  void IndirectIndexed_Write(uint8_t &input, BYTE index);
  void ZeroPage_Modify(OPERATION operation);
  void ZeroPage_Modify(OPERATION operation, BYTE index);
  void ZeroPage_Read(OPERATION operation, BYTE &output);
  void ZeroPage_Read(OPERATION operation, BYTE &output, BYTE index);
  void ZeroPage_Write(BYTE &input);
  void ZeroPage_Write(BYTE &input, BYTE index);

  //
  // Operations
  //

  void ADC(const BYTE input, BYTE &output);
  void AND(const BYTE input, BYTE &output);
  void ASL(const BYTE input, BYTE &output);
  void BIT(const BYTE input, BYTE &output);
  void CMP(const BYTE input, BYTE &output);
  void DEC(const BYTE input, BYTE &output);
  void EOR(const BYTE input, BYTE &output);
  void INC(const BYTE input, BYTE &output);
  void LDx(const BYTE input, BYTE &output);
  void LSR(const BYTE input, BYTE &output);
  void ORA(const BYTE input, BYTE &output);
  void ROL(const BYTE input, BYTE &output);
  void ROR(const BYTE input, BYTE &output);
  void SBC(const BYTE input, BYTE &output);

};
