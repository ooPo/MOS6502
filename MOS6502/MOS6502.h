//
// MOS6502.h
// by Naomi Peori (naomi@peori.ca)
//

#pragma once
#include <array>
#include <cstdint>

class MOS6502
{

public:

  using BYTE = uint8_t;

  // Reg16 provides convenient byte-lane access to a 16-bit value.
  // The 6502 and the NES always store 16-bit values little-endian, and
  // virtually every modern host is also little-endian, so we rely on that
  // here.  If you ever need to port to a big-endian host, swap l/h below.
  //
  // Note: accessing the individual byte members of a union through a
  // different member than the one last written is technically
  // implementation-defined in C++, but is well-defined on every compiler
  // used for NES emulation (GCC, Clang, MSVC).
  union Reg16 {
    uint16_t w;
    struct { uint8_t l, h; };

    // Allow simple assignment from a raw uint16_t.
    Reg16& operator=(uint16_t v) { w = v; return *this; }
  };

  using WORD = Reg16;

  enum INTERRUPT { NMI = 0, IRQ = 1, COUNT = 2 };

  void Run();

  void Halt()   { running = false; }
  void Signal(const INTERRUPT interrupt, const bool value) { signals[interrupt] = value; }

  // On a real 6502, RESET holds the R/W line high during the three stack
  // cycles, so SP decrements but no data is written.  We model that by
  // decrementing SP directly instead of calling Push().
  void Reset() {
    S   -= 3;
    PC.l = Load(0xFFFC);
    PC.h = Load(0xFFFD);
    P.I  = 1;
  }

  virtual uint8_t Load(uint16_t address) = 0;
  virtual void    Store(uint16_t address, uint8_t value) = 0;

  // Optional callback invoked when an unknown opcode is encountered.
  // Override to add logging or halt behaviour.  Default: no-op.
  virtual void OnUnknownOpcode(uint8_t opcode) {}

protected:

  bool enableBCD = false;

  bool running = false;

  // IRQ is level-triggered: the external device holds the line asserted.
  // NMI is edge-triggered: the emulator clears it after acknowledging.
  // Use Signal() to assert/de-assert from outside the CPU.
  std::array<bool, INTERRUPT::COUNT> signals = {};

  WORD PC = { .w = 0x0000 };
  WORD AB = { .w = 0x0000 };
  WORD TB = { .w = 0x0000 };

  BYTE A = 0x00;
  BYTE X = 0x00;
  BYTE Y = 0x00;
  BYTE S = 0xFF;

  // Processor status register.
  // Bit layout matches the real 6502: C Z I D B T V N (low to high).
  union {
    BYTE value;
    struct { uint8_t C:1, Z:1, I:1, D:1, B:1, T:1, V:1, N:1; };
  }
  P = { .value = 0x34 };

protected:

  //
  // Helpers
  //

  // Branch: fetch the signed offset, then conditionally take it.
  // When taken: one unconditional idle cycle while the ALU adds the offset,
  // then a second conditional idle cycle if the result crossed a page boundary.
  inline void Branch(bool test) {
    const int8_t offset = static_cast<int8_t>(Fetch());
    if (test) {
      Idle();                          // cycle 3: unconditional taken penalty
      IdleOnPageCrossed(PC, offset);   // cycle 4: conditional page-cross penalty
      PC.w += offset;
    }
  }

  // Fetch the byte at PC and advance PC.
  inline uint8_t Fetch() {
    return Load(PC.w++);
  }

  // Set N and Z from value, then return it unchanged.
  inline uint8_t Flags(uint8_t value) {
    P.N = (value & 0x80) ? 1 : 0;
    P.Z = (value == 0x00) ? 1 : 0;
    return value;
  }

  // Consume one idle cycle (dummy read at PC, without advancing it).
  inline void Idle() {
    Load(PC.w);
  }

  // Consume one idle cycle at the top byte of the unindexed address
  // combined with the low byte of the effective address.  Used for
  // write and read-modify-write modes that always pay the extra cycle.
  inline WORD IdleOnPageAlways(WORD base, BYTE index) {
    TB.w = base.w + index;
    Load((base.h << 8) | TB.l); // dummy read on the un-fixed page
    return TB;
  }

  // Same as IdleOnPageAlways, but only consume the cycle when the
  // addition actually crossed a page boundary.  Used for read modes.
  inline WORD IdleOnPageCrossed(WORD base, BYTE index) {
    TB.w = base.w + index;
    if (base.l > TB.l) { Load((base.h << 8) | TB.l); } // dummy read
    return TB;
  }

  // Shared hardware interrupt sequence (NMI / IRQ).
  // 7-cycle sequence: the two idle cycles here represent the internal operations
  // on cycles 2 and 3 before the stack pushes begin.
  // The B flag is NOT set in the pushed status byte for hardware interrupts;
  // BRK sets it explicitly in its own handler.
  inline void Interrupt(uint16_t vector) {
    Idle();              // cycle 2: internal operation
    Idle();              // cycle 3: internal operation
    Push(PC.h);
    Push(PC.l);
    Push(P.value | 0x20); // bit 5 (T/unused) always pushed as 1
    PC.l = Load(vector);
    PC.h = Load(vector + 1);
    P.I  = 1;
  }

  // Stack helpers: the 6502 stack lives at $0100–$01FF.
  inline uint8_t Pull() {
    return Load(0x0100 | ++S);
  }

  inline void Push(uint8_t value) {
    Store(0x0100 | S--, value);
  }

  // Consume one idle cycle reading the current stack pointer without
  // modifying S.  Used by RTS and the pull instructions.
  inline void IdleStack() {
    Load(0x0100 | S);
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
  // Convention: `input` is the operand fetched from memory (or immediate).
  // `output` is the destination register passed by reference.  For
  // accumulator operations (AND, EOR, ORA, ADC, SBC, LDx) `output` is
  // always the relevant register (A/X/Y).  For memory-mutating operations
  // (ASL, LSR, ROL, ROR, INC, DEC) `output` receives the result that will
  // be written back to memory.
  //
  // CMP and BIT are special: they only affect flags and never write a
  // result; `output` is the register to compare against for CMP, and is
  // unused for BIT.
  //

  void ADC(BYTE input, BYTE &output);
  void AND(BYTE input, BYTE &output);
  void ASL(BYTE input, BYTE &output);
  void BIT(BYTE input, BYTE &output);
  void CMP(BYTE input, BYTE &output); // output = register to compare against
  void DEC(BYTE input, BYTE &output);
  void EOR(BYTE input, BYTE &output);
  void INC(BYTE input, BYTE &output);
  void LDx(BYTE input, BYTE &output);
  void LSR(BYTE input, BYTE &output);
  void ORA(BYTE input, BYTE &output);
  void ROL(BYTE input, BYTE &output);
  void ROR(BYTE input, BYTE &output);
  void SBC(BYTE input, BYTE &output);

};
