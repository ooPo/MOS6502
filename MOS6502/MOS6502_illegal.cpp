//
// MOS6502_illegal.cpp
// by Naomi Peori (naomi@peori.ca)
//

#include "MOS6502.h"

//
// Illegal Opcode Dispatch
//

void MOS6502::RunIllegal(BYTE opcode) {
  switch (opcode) {

    // ---------------------------------------------------------------
    // ANC — AND #imm, then copy N to C
    // ---------------------------------------------------------------

    case 0x0B: case 0x2B:
      Immediate_Read(&MOS6502::ANC, A);
      break;

    // ---------------------------------------------------------------
    // ALR — AND #imm, then LSR A
    // ---------------------------------------------------------------

    case 0x4B:
      Immediate_Read(&MOS6502::ALR, A);
      break;

    // ---------------------------------------------------------------
    // ARR — AND #imm, then ROR A (special C/V flags)
    // ---------------------------------------------------------------

    case 0x6B:
      Immediate_Read(&MOS6502::ARR, A);
      break;

    // ---------------------------------------------------------------
    // AXS — (A & X) - #imm → X, CMP-like flags
    // ---------------------------------------------------------------

    case 0xCB:
      Immediate_Read(&MOS6502::AXS, X);
      break;

    // ---------------------------------------------------------------
    // SBC — duplicate of official SBC immediate
    // ---------------------------------------------------------------

    case 0xEB:
      Immediate_Read(&MOS6502::SBC, A);
      break;

    // ---------------------------------------------------------------
    // LAS — mem & S → A, X, S
    // ---------------------------------------------------------------

    case 0xBB:
      Absolute_Read(&MOS6502::LAS, A, Y);
      break;

    // ---------------------------------------------------------------
    // SHY — Store Y & (addr_high + 1), Absolute, X
    // ---------------------------------------------------------------

    case 0x9C: {
      AB.l = Fetch();
      AB.h = Fetch();
      BYTE v = Y & (AB.h + 1);
      AB = IdleOnPageAlways(AB, X);
      Store(AB.w, v);
      break;
    }

    // ---------------------------------------------------------------
    // SHX — Store X & (addr_high + 1), Absolute, Y
    // ---------------------------------------------------------------

    case 0x9E: {
      AB.l = Fetch();
      AB.h = Fetch();
      BYTE v = X & (AB.h + 1);
      AB = IdleOnPageAlways(AB, Y);
      Store(AB.w, v);
      break;
    }

    // ---------------------------------------------------------------
    // TAS — S = A & X; store S & (addr_high + 1), Absolute, Y
    // ---------------------------------------------------------------

    case 0x9B: {
      AB.l = Fetch();
      AB.h = Fetch();
      S = A & X;
      BYTE v = S & (AB.h + 1);
      AB = IdleOnPageAlways(AB, Y);
      Store(AB.w, v);
      break;
    }

    // ---------------------------------------------------------------
    // DCP — DEC, then CMP
    // ---------------------------------------------------------------

    case 0xCF: Absolute_Modify        (&MOS6502::DCP);       break;
    case 0xDF: Absolute_Modify        (&MOS6502::DCP, X);    break;
    case 0xDB: Absolute_Modify        (&MOS6502::DCP, Y);    break;
    case 0xC7: ZeroPage_Modify        (&MOS6502::DCP);       break;
    case 0xD7: ZeroPage_Modify        (&MOS6502::DCP, X);    break;
    case 0xC3: IndexedIndirect_Modify (&MOS6502::DCP, X);    break;
    case 0xD3: IndirectIndexed_Modify (&MOS6502::DCP, Y);    break;

    // ---------------------------------------------------------------
    // ISC — INC, then SBC
    // ---------------------------------------------------------------

    case 0xEF: Absolute_Modify        (&MOS6502::ISC);       break;
    case 0xFF: Absolute_Modify        (&MOS6502::ISC, X);    break;
    case 0xFB: Absolute_Modify        (&MOS6502::ISC, Y);    break;
    case 0xE7: ZeroPage_Modify        (&MOS6502::ISC);       break;
    case 0xF7: ZeroPage_Modify        (&MOS6502::ISC, X);    break;
    case 0xE3: IndexedIndirect_Modify (&MOS6502::ISC, X);    break;
    case 0xF3: IndirectIndexed_Modify (&MOS6502::ISC, Y);    break;

    // ---------------------------------------------------------------
    // LAX — Load A and X
    // ---------------------------------------------------------------

    case 0xAF: Absolute_Read          (&MOS6502::LAX, A);    break;
    case 0xBF: Absolute_Read          (&MOS6502::LAX, A, Y); break;
    case 0xA7: ZeroPage_Read          (&MOS6502::LAX, A);    break;
    case 0xB7: ZeroPage_Read          (&MOS6502::LAX, A, Y); break;
    case 0xA3: IndexedIndirect_Read   (&MOS6502::LAX, A, X); break;
    case 0xB3: IndirectIndexed_Read   (&MOS6502::LAX, A, Y); break;

    // ---------------------------------------------------------------
    // RLA — ROL, then AND
    // ---------------------------------------------------------------

    case 0x2F: Absolute_Modify        (&MOS6502::RLA);       break;
    case 0x3F: Absolute_Modify        (&MOS6502::RLA, X);    break;
    case 0x3B: Absolute_Modify        (&MOS6502::RLA, Y);    break;
    case 0x27: ZeroPage_Modify        (&MOS6502::RLA);       break;
    case 0x37: ZeroPage_Modify        (&MOS6502::RLA, X);    break;
    case 0x23: IndexedIndirect_Modify (&MOS6502::RLA, X);    break;
    case 0x33: IndirectIndexed_Modify (&MOS6502::RLA, Y);    break;

    // ---------------------------------------------------------------
    // RRA — ROR, then ADC
    // ---------------------------------------------------------------

    case 0x6F: Absolute_Modify        (&MOS6502::RRA);       break;
    case 0x7F: Absolute_Modify        (&MOS6502::RRA, X);    break;
    case 0x7B: Absolute_Modify        (&MOS6502::RRA, Y);    break;
    case 0x67: ZeroPage_Modify        (&MOS6502::RRA);       break;
    case 0x77: ZeroPage_Modify        (&MOS6502::RRA, X);    break;
    case 0x63: IndexedIndirect_Modify (&MOS6502::RRA, X);    break;
    case 0x73: IndirectIndexed_Modify (&MOS6502::RRA, Y);    break;

    // ---------------------------------------------------------------
    // SAX — Store A & X
    // ---------------------------------------------------------------

    case 0x8F: { BYTE v = A & X; Absolute_Write        (v);    break; }
    case 0x87: { BYTE v = A & X; ZeroPage_Write        (v);    break; }
    case 0x97: { BYTE v = A & X; ZeroPage_Write        (v, Y); break; }
    case 0x83: { BYTE v = A & X; IndexedIndirect_Write (v, X); break; }

    // ---------------------------------------------------------------
    // SLO — ASL, then ORA
    // ---------------------------------------------------------------

    case 0x0F: Absolute_Modify        (&MOS6502::SLO);       break;
    case 0x1F: Absolute_Modify        (&MOS6502::SLO, X);    break;
    case 0x1B: Absolute_Modify        (&MOS6502::SLO, Y);    break;
    case 0x07: ZeroPage_Modify        (&MOS6502::SLO);       break;
    case 0x17: ZeroPage_Modify        (&MOS6502::SLO, X);    break;
    case 0x03: IndexedIndirect_Modify (&MOS6502::SLO, X);    break;
    case 0x13: IndirectIndexed_Modify (&MOS6502::SLO, Y);    break;

    // ---------------------------------------------------------------
    // SRE — LSR, then EOR
    // ---------------------------------------------------------------

    case 0x4F: Absolute_Modify        (&MOS6502::SRE);       break;
    case 0x5F: Absolute_Modify        (&MOS6502::SRE, X);    break;
    case 0x5B: Absolute_Modify        (&MOS6502::SRE, Y);    break;
    case 0x47: ZeroPage_Modify        (&MOS6502::SRE);       break;
    case 0x57: ZeroPage_Modify        (&MOS6502::SRE, X);    break;
    case 0x43: IndexedIndirect_Modify (&MOS6502::SRE, X);    break;
    case 0x53: IndirectIndexed_Modify (&MOS6502::SRE, Y);    break;

    // ---------------------------------------------------------------
    // NOP Variants
    // ---------------------------------------------------------------

    // Implied
    case 0x1A: case 0x3A: case 0x5A:
    case 0x7A: case 0xDA: case 0xFA:
      Idle();
      break;

    // Immediate
    case 0x80: case 0x82: case 0x89:
    case 0xC2: case 0xE2:
      Fetch();
      break;

    // Zero Page
    case 0x04: case 0x44: case 0x64:
      ZeroPage_Read(&MOS6502::NOP, A);
      break;

    // Zero Page, X
    case 0x14: case 0x34: case 0x54:
    case 0x74: case 0xD4: case 0xF4:
      ZeroPage_Read(&MOS6502::NOP, A, X);
      break;

    // Absolute
    case 0x0C:
      Absolute_Read(&MOS6502::NOP, A);
      break;

    // Absolute, X
    case 0x1C: case 0x3C: case 0x5C:
    case 0x7C: case 0xDC: case 0xFC:
      Absolute_Read(&MOS6502::NOP, A, X);
      break;

    // ---------------------------------------------------------------
    // Unknown
    // ---------------------------------------------------------------

    default:
      OnUnknownOpcode(opcode);
      break;
  }
}

//
// Illegal Addressing Modes
//

void MOS6502::IndexedIndirect_Modify(OPERATION operation, BYTE index) {
  TB.w  = Fetch();
  Load(TB.w);
  TB.l += index;
  AB.l  = Load(TB.w);
  TB.l += 1;
  AB.h  = Load(TB.w);
  BYTE input = Load(AB.w);
  Store(AB.w, input);
  BYTE output = 0;
  std::invoke(operation, this, input, output);
  Store(AB.w, output);
}

void MOS6502::IndirectIndexed_Modify(OPERATION operation, BYTE index) {
  TB.w  = Fetch();
  AB.l  = Load(TB.w);
  TB.l += 1;
  AB.h  = Load(TB.w);
  AB    = IdleOnPageAlways(AB, index);
  BYTE input = Load(AB.w);
  Store(AB.w, input);
  BYTE output = 0;
  std::invoke(operation, this, input, output);
  Store(AB.w, output);
}

//
// Illegal Operations
//

void MOS6502::DCP(BYTE input, BYTE &output) { DEC(input, output); CMP(output, A); }
void MOS6502::ISC(BYTE input, BYTE &output) { INC(input, output); SBC(output, A); }
void MOS6502::RLA(BYTE input, BYTE &output) { ROL(input, output); AND(output, A); }
void MOS6502::RRA(BYTE input, BYTE &output) { ROR(input, output); ADC(output, A); }
void MOS6502::SLO(BYTE input, BYTE &output) { ASL(input, output); ORA(output, A); }
void MOS6502::SRE(BYTE input, BYTE &output) { LSR(input, output); EOR(output, A); }

void MOS6502::ALR(BYTE input, BYTE &output) {
  AND(input, output);
  LSR(output, output);
}

void MOS6502::ANC(BYTE input, BYTE &output) {
  AND(input, output);
  P.C = P.N;
}

void MOS6502::ARR(BYTE input, BYTE &output) {
  AND(input, output);
  output = (output >> 1) | (P.C << 7);
  Flags(output);
  P.C = (output & 0x40) ? 1 : 0;
  P.V = ((output ^ (output << 1)) & 0x40) ? 1 : 0;
}

void MOS6502::AXS(BYTE input, BYTE &output) {
  uint16_t temp = (A & X) - input;
  P.C = (temp < 0x100) ? 1 : 0;
  output = Flags(temp & 0xFF);
}

void MOS6502::LAS(BYTE input, [[maybe_unused]] BYTE &output) {
  A = X = S = Flags(input & S);
}

void MOS6502::LAX(BYTE input, [[maybe_unused]] BYTE &output) {
  A = Flags(input); X = A;
}

void MOS6502::NOP([[maybe_unused]] BYTE input, [[maybe_unused]] BYTE &output) { }
