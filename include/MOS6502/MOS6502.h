//
// MOS6502.h
// by Naomi Peori (naomi@peori.ca)
//

#pragma once
#include <array>
#include <cstdint>
#include <functional>

class MOS6502
{

public:

  union Reg16 {
    uint16_t w;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    struct { uint8_t h, l; };
#else
    struct { uint8_t l, h; };
#endif
    Reg16 &operator=(uint16_t v) { w = v; return *this; }
  };

  static_assert(sizeof(Reg16) == 2, "Reg16 must be exactly 2 bytes");

  using WORD = Reg16;
  using BYTE = uint8_t;

  // B (bit 4) and U (bit 5), set when BRK/PHP pushes P
  static constexpr uint8_t P_BT_MASK = 0x30;

  void Run();
  void Halt() { running = false; }

  enum INTERRUPT { NMI = 0, IRQ = 1, COUNT };
  void Signal(const INTERRUPT interrupt, bool value) { signals[interrupt] = value; }

  void Reset() {
    S   -= 3;
    PC.l = Load(0xFFFC);
    PC.h = Load(0xFFFD);
    P.I  = 1;
  }

  virtual uint8_t Load(uint16_t address, bool peek = false) = 0;
  virtual void Store(uint16_t address, uint8_t value) = 0;

  virtual void OnUnknownOpcode(uint8_t) {}

protected:

  bool enableBCD = true;
  bool enableIllegal = false;

private:

  bool running = false;

  std::array<bool, INTERRUPT::COUNT> signals = {};

  WORD PC = { .w = 0x0000 };
  WORD AB = { .w = 0x0000 };
  WORD TB = { .w = 0x0000 };

  BYTE A = 0x00;
  BYTE X = 0x00;
  BYTE Y = 0x00;
  BYTE S = 0xFF;

  union {
    BYTE value;
    struct { uint8_t C:1, Z:1, I:1, D:1, B:1, U:1, V:1, N:1; };
  } P = { .value = 0x34 };

  static_assert(sizeof(P) == 1, "P register bitfield must be exactly 1 byte; bit ordering assumes LSB-first packing (GCC/Clang default)");

  //
  // Helpers
  //

  inline void Branch(bool test) {
    const int8_t offset = static_cast<int8_t>(Fetch());

    if (test) {
      Idle();
      const uint16_t target = static_cast<uint16_t>(PC.w + offset);
      if ((PC.w ^ target) & 0xFF00) { Load((PC.h << 8) | (target & 0xFF)); }
      PC.w = target;
    }
  }

  inline uint8_t Fetch() {
    return Load(PC.w++);
  }

  inline uint8_t Flags(uint8_t value) {
    P.N = (value & 0x80) ? 1 : 0;
    P.Z = (value == 0x00) ? 1 : 0;
    return value;
  }

  inline void Idle() {
    Load(PC.w);
  }
  
  inline void IdleStack() {
    Load(0x0100 | S);
  }

  inline WORD IdleOnPageAlways(WORD base, BYTE index) {
    TB.w = base.w + index;
    Load((base.h << 8) | TB.l);
    return TB;
  }

  inline WORD IdleOnPageCrossed(WORD base, BYTE index) {
    TB.w = base.w + index;
    if (base.l > TB.l) { Load((base.h << 8) | TB.l); }
    return TB;
  }

  inline void DispatchInterrupt(uint16_t vector) {
    Idle();
    Idle();
    Push(PC.h);
    Push(PC.l);
    Push(P.value | 0x20);
    PC.l = Load(vector);
    PC.h = Load(vector + 1);
    P.I  = 1;
  }

  [[nodiscard]] inline uint8_t Pull() {
    return Load(0x0100 | ++S);
  }

  inline void Push(uint8_t value) {
    Store(0x0100 | S--, value);
  }

  //
  // Addressing Modes
  //

  using OPERATION = void (MOS6502::*)(BYTE input, BYTE &output);

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
  void IndirectIndexed_Write(BYTE &input, BYTE index);
  void ZeroPage_Modify(OPERATION operation);
  void ZeroPage_Modify(OPERATION operation, BYTE index);
  void ZeroPage_Read(OPERATION operation, BYTE &output);
  void ZeroPage_Read(OPERATION operation, BYTE &output, BYTE index);
  void ZeroPage_Write(BYTE &input);
  void ZeroPage_Write(BYTE &input, BYTE index);

  //
  // Operations
  //

  void ADC(BYTE input, BYTE &output);
  void AND(BYTE input, BYTE &output);
  void ASL(BYTE input, BYTE &output);
  void BIT(BYTE input, BYTE &output);
  void CMP(BYTE input, BYTE &output);
  void DEC(BYTE input, BYTE &output);
  void EOR(BYTE input, BYTE &output);
  void INC(BYTE input, BYTE &output);
  void LDx(BYTE input, BYTE &output);
  void LSR(BYTE input, BYTE &output);
  void ORA(BYTE input, BYTE &output);
  void ROL(BYTE input, BYTE &output);
  void ROR(BYTE input, BYTE &output);
  void SBC(BYTE input, BYTE &output);

  //
  // Illegal Addressing Modes
  //

  void IndexedIndirect_Modify(OPERATION operation, BYTE index);
  void IndirectIndexed_Modify(OPERATION operation, BYTE index);

  //
  // Illegal Operations
  //

  void RunIllegal(BYTE opcode);

  void ANC(BYTE input, BYTE &output);
  void ALR(BYTE input, BYTE &output);
  void ARR(BYTE input, BYTE &output);
  void AXS(BYTE input, BYTE &output);
  void DCP(BYTE input, BYTE &output);
  void ISC(BYTE input, BYTE &output);
  void LAX(BYTE input, BYTE &output);
  void LAS(BYTE input, BYTE &output);
  void NOP(BYTE input, BYTE &output);
  void RLA(BYTE input, BYTE &output);
  void RRA(BYTE input, BYTE &output);
  void SLO(BYTE input, BYTE &output);
  void SRE(BYTE input, BYTE &output);

};
