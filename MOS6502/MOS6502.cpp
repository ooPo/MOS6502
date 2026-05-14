//
// MOS6502.cpp
// by Naomi Peori (naomi@peori.ca)
//

#include "MOS6502.h"

void MOS6502::Run() {
  running = true;

  while (running) {

    // NMI is edge-triggered; clear the latch on acknowledge.
    if (signals[INTERRUPT::NMI]) {
      signals[INTERRUPT::NMI] = false;
      Interrupt(0xFFFA);
    }

    // IRQ is level-triggered; the device de-asserts it, not the CPU.
    else if (!P.I && signals[INTERRUPT::IRQ]) {
      Interrupt(0xFFFE);
    }

    const BYTE opcode = Fetch();

    //
    // Opcode Dispatch
    //

    switch (opcode) {

      // ---------------------------------------------------------------
      // ADC
      // ---------------------------------------------------------------

      case 0x6D: Absolute_Read        (&MOS6502::ADC, A);    break;
      case 0x7D: Absolute_Read        (&MOS6502::ADC, A, X); break;
      case 0x79: Absolute_Read        (&MOS6502::ADC, A, Y); break;
      case 0x69: Immediate_Read       (&MOS6502::ADC, A);    break;
      case 0x61: IndexedIndirect_Read (&MOS6502::ADC, A, X); break;
      case 0x71: IndirectIndexed_Read (&MOS6502::ADC, A, Y); break;
      case 0x65: ZeroPage_Read        (&MOS6502::ADC, A);    break;
      case 0x75: ZeroPage_Read        (&MOS6502::ADC, A, X); break;

      // ---------------------------------------------------------------
      // AND
      // ---------------------------------------------------------------

      case 0x2D: Absolute_Read        (&MOS6502::AND, A);    break;
      case 0x3D: Absolute_Read        (&MOS6502::AND, A, X); break;
      case 0x39: Absolute_Read        (&MOS6502::AND, A, Y); break;
      case 0x29: Immediate_Read       (&MOS6502::AND, A);    break;
      case 0x21: IndexedIndirect_Read (&MOS6502::AND, A, X); break;
      case 0x31: IndirectIndexed_Read (&MOS6502::AND, A, Y); break;
      case 0x25: ZeroPage_Read        (&MOS6502::AND, A);    break;
      case 0x35: ZeroPage_Read        (&MOS6502::AND, A, X); break;

      // ---------------------------------------------------------------
      // ASL
      // ---------------------------------------------------------------

      case 0x0E: Absolute_Modify      (&MOS6502::ASL);       break;
      case 0x1E: Absolute_Modify      (&MOS6502::ASL, X);    break;
      case 0x06: ZeroPage_Modify      (&MOS6502::ASL);       break;
      case 0x16: ZeroPage_Modify      (&MOS6502::ASL, X);    break;

      case 0x0A: Idle(); ASL(A, A); break; // Accumulator

      // ---------------------------------------------------------------
      // Branch Operations
      // ---------------------------------------------------------------

      case 0x90: Branch(!P.C); break; // BCC
      case 0xB0: Branch( P.C); break; // BCS
      case 0xF0: Branch( P.Z); break; // BEQ
      case 0x30: Branch( P.N); break; // BMI
      case 0xD0: Branch(!P.Z); break; // BNE
      case 0x10: Branch(!P.N); break; // BPL
      case 0x50: Branch(!P.V); break; // BVC
      case 0x70: Branch( P.V); break; // BVS

      // ---------------------------------------------------------------
      // BIT
      // ---------------------------------------------------------------

      case 0x2C: Absolute_Read        (&MOS6502::BIT, A);    break;
      case 0x24: ZeroPage_Read        (&MOS6502::BIT, A);    break;

      // ---------------------------------------------------------------
      // BRK
      // ---------------------------------------------------------------

      case 0x00:
        Fetch();
        Push(PC.h);
        Push(PC.l);
        Push(P.value | 0x30); // B and T set in pushed byte
        PC.l = Load(0xFFFE);
        PC.h = Load(0xFFFF);
        P.I  = 1;
        break;

      // ---------------------------------------------------------------
      // Flag Operations
      // ---------------------------------------------------------------

      case 0x18: Idle(); P.C = 0; break; // CLC
      case 0xD8: Idle(); P.D = 0; break; // CLD
      case 0x58: Idle(); P.I = 0; break; // CLI
      case 0xB8: Idle(); P.V = 0; break; // CLV
      case 0x38: Idle(); P.C = 1; break; // SEC
      case 0xF8: Idle(); P.D = 1; break; // SED
      case 0x78: Idle(); P.I = 1; break; // SEI

      // ---------------------------------------------------------------
      // CMP / CPX / CPY
      // ---------------------------------------------------------------

      case 0xCD: Absolute_Read        (&MOS6502::CMP, A);    break;
      case 0xDD: Absolute_Read        (&MOS6502::CMP, A, X); break;
      case 0xD9: Absolute_Read        (&MOS6502::CMP, A, Y); break;
      case 0xC9: Immediate_Read       (&MOS6502::CMP, A);    break;
      case 0xC1: IndexedIndirect_Read (&MOS6502::CMP, A, X); break;
      case 0xD1: IndirectIndexed_Read (&MOS6502::CMP, A, Y); break;
      case 0xC5: ZeroPage_Read        (&MOS6502::CMP, A);    break;
      case 0xD5: ZeroPage_Read        (&MOS6502::CMP, A, X); break;

      case 0xEC: Absolute_Read        (&MOS6502::CMP, X);    break;
      case 0xE0: Immediate_Read       (&MOS6502::CMP, X);    break;
      case 0xE4: ZeroPage_Read        (&MOS6502::CMP, X);    break;

      case 0xCC: Absolute_Read        (&MOS6502::CMP, Y);    break;
      case 0xC0: Immediate_Read       (&MOS6502::CMP, Y);    break;
      case 0xC4: ZeroPage_Read        (&MOS6502::CMP, Y);    break;

      // ---------------------------------------------------------------
      // DEC / DEX / DEY
      // ---------------------------------------------------------------

      case 0xCE: Absolute_Modify      (&MOS6502::DEC);       break;
      case 0xDE: Absolute_Modify      (&MOS6502::DEC, X);    break;
      case 0xC6: ZeroPage_Modify      (&MOS6502::DEC);       break;
      case 0xD6: ZeroPage_Modify      (&MOS6502::DEC, X);    break;

      case 0xCA: Idle(); X = Flags(X - 1); break; // DEX
      case 0x88: Idle(); Y = Flags(Y - 1); break; // DEY

      // ---------------------------------------------------------------
      // EOR
      // ---------------------------------------------------------------

      case 0x4D: Absolute_Read        (&MOS6502::EOR, A);    break;
      case 0x5D: Absolute_Read        (&MOS6502::EOR, A, X); break;
      case 0x59: Absolute_Read        (&MOS6502::EOR, A, Y); break;
      case 0x49: Immediate_Read       (&MOS6502::EOR, A);    break;
      case 0x41: IndexedIndirect_Read (&MOS6502::EOR, A, X); break;
      case 0x51: IndirectIndexed_Read (&MOS6502::EOR, A, Y); break;
      case 0x45: ZeroPage_Read        (&MOS6502::EOR, A);    break;
      case 0x55: ZeroPage_Read        (&MOS6502::EOR, A, X); break;

      // ---------------------------------------------------------------
      // INC / INX / INY
      // ---------------------------------------------------------------

      case 0xEE: Absolute_Modify      (&MOS6502::INC);       break;
      case 0xFE: Absolute_Modify      (&MOS6502::INC, X);    break;
      case 0xE6: ZeroPage_Modify      (&MOS6502::INC);       break;
      case 0xF6: ZeroPage_Modify      (&MOS6502::INC, X);    break;

      case 0xE8: Idle(); X = Flags(X + 1); break; // INX
      case 0xC8: Idle(); Y = Flags(Y + 1); break; // INY

      // ---------------------------------------------------------------
      // JMP
      // ---------------------------------------------------------------

      case 0x4C: // Absolute
        AB.l = Fetch();
        AB.h = Fetch();
        PC   = AB;
        break;

      case 0x6C: // Indirect
        AB.l  = Fetch();
        AB.h  = Fetch();
        PC.l  = Load(AB.w);
        AB.l += 1;
        PC.h  = Load(AB.w);
        break;

      // ---------------------------------------------------------------
      // JSR
      // ---------------------------------------------------------------

      case 0x20:
        AB.l = Fetch();
        IdleStack();
        Push(PC.h);
        Push(PC.l);
        AB.h = Fetch();
        PC   = AB;
        break;

      // ---------------------------------------------------------------
      // LDA / LDX / LDY
      // ---------------------------------------------------------------

      case 0xAD: Absolute_Read        (&MOS6502::LDx, A);    break;
      case 0xBD: Absolute_Read        (&MOS6502::LDx, A, X); break;
      case 0xB9: Absolute_Read        (&MOS6502::LDx, A, Y); break;
      case 0xA9: Immediate_Read       (&MOS6502::LDx, A);    break;
      case 0xA1: IndexedIndirect_Read (&MOS6502::LDx, A, X); break;
      case 0xB1: IndirectIndexed_Read (&MOS6502::LDx, A, Y); break;
      case 0xA5: ZeroPage_Read        (&MOS6502::LDx, A);    break;
      case 0xB5: ZeroPage_Read        (&MOS6502::LDx, A, X); break;

      case 0xAE: Absolute_Read        (&MOS6502::LDx, X);    break;
      case 0xBE: Absolute_Read        (&MOS6502::LDx, X, Y); break;
      case 0xA2: Immediate_Read       (&MOS6502::LDx, X);    break;
      case 0xA6: ZeroPage_Read        (&MOS6502::LDx, X);    break;
      case 0xB6: ZeroPage_Read        (&MOS6502::LDx, X, Y); break;

      case 0xAC: Absolute_Read        (&MOS6502::LDx, Y);    break;
      case 0xBC: Absolute_Read        (&MOS6502::LDx, Y, X); break;
      case 0xA0: Immediate_Read       (&MOS6502::LDx, Y);    break;
      case 0xA4: ZeroPage_Read        (&MOS6502::LDx, Y);    break;
      case 0xB4: ZeroPage_Read        (&MOS6502::LDx, Y, X); break;

      // ---------------------------------------------------------------
      // LSR
      // ---------------------------------------------------------------

      case 0x4E: Absolute_Modify      (&MOS6502::LSR);    break;
      case 0x5E: Absolute_Modify      (&MOS6502::LSR, X); break;
      case 0x46: ZeroPage_Modify      (&MOS6502::LSR);    break;
      case 0x56: ZeroPage_Modify      (&MOS6502::LSR, X); break;

      case 0x4A: Idle(); LSR(A, A); break; // Accumulator

      // ---------------------------------------------------------------
      // NOP
      // ---------------------------------------------------------------

      case 0xEA: Idle(); break;

      // ---------------------------------------------------------------
      // ORA
      // ---------------------------------------------------------------

      case 0x0D: Absolute_Read        (&MOS6502::ORA, A);    break;
      case 0x1D: Absolute_Read        (&MOS6502::ORA, A, X); break;
      case 0x19: Absolute_Read        (&MOS6502::ORA, A, Y); break;
      case 0x09: Immediate_Read       (&MOS6502::ORA, A);    break;
      case 0x01: IndexedIndirect_Read (&MOS6502::ORA, A, X); break;
      case 0x11: IndirectIndexed_Read (&MOS6502::ORA, A, Y); break;
      case 0x05: ZeroPage_Read        (&MOS6502::ORA, A);    break;
      case 0x15: ZeroPage_Read        (&MOS6502::ORA, A, X); break;

      // ---------------------------------------------------------------
      // PHA / PHP / PLA / PLP
      // ---------------------------------------------------------------

      case 0x48: // PHA
        Idle();
        Push(A);
        break;
 
      case 0x08: // PHP
        Idle();
        Push(P.value | 0x30);
        break;

      case 0x68: // PLA
        Idle(); IdleStack();
        A = Flags(Pull());
        break;

      case 0x28: // PLP
        Idle(); IdleStack();
        P.value  = Pull();
        P.value &= ~0x10;
        P.value |=  0x20;
        break;

      // ---------------------------------------------------------------
      // ROL
      // ---------------------------------------------------------------

      case 0x2E: Absolute_Modify      (&MOS6502::ROL);       break;
      case 0x3E: Absolute_Modify      (&MOS6502::ROL, X);    break;
      case 0x26: ZeroPage_Modify      (&MOS6502::ROL);       break;
      case 0x36: ZeroPage_Modify      (&MOS6502::ROL, X);    break;

      case 0x2A: Idle(); ROL(A, A); break; // Accumulator

      // ---------------------------------------------------------------
      // ROR
      // ---------------------------------------------------------------

      case 0x6E: Absolute_Modify      (&MOS6502::ROR);       break;
      case 0x7E: Absolute_Modify      (&MOS6502::ROR, X);    break;
      case 0x66: ZeroPage_Modify      (&MOS6502::ROR);       break;
      case 0x76: ZeroPage_Modify      (&MOS6502::ROR, X);    break;

      case 0x6A: Idle(); ROR(A, A); break; // Accumulator

      // ---------------------------------------------------------------
      // RTI / RTS
      // ---------------------------------------------------------------

      case 0x40: // RTI
        Idle(); IdleStack();
        P.value  = Pull();
        P.value &= ~0x10;
        P.value |=  0x20;
        PC.l     = Pull();
        PC.h     = Pull();
        break;

      case 0x60: // RTS
        Idle(); IdleStack();
        PC.l = Pull();
        PC.h = Pull();
        Fetch();
        break;

      // ---------------------------------------------------------------
      // SBC
      // ---------------------------------------------------------------

      case 0xED: Absolute_Read        (&MOS6502::SBC, A);    break;
      case 0xFD: Absolute_Read        (&MOS6502::SBC, A, X); break;
      case 0xF9: Absolute_Read        (&MOS6502::SBC, A, Y); break;
      case 0xE9: Immediate_Read       (&MOS6502::SBC, A);    break;
      case 0xE1: IndexedIndirect_Read (&MOS6502::SBC, A, X); break;
      case 0xF1: IndirectIndexed_Read (&MOS6502::SBC, A, Y); break;
      case 0xE5: ZeroPage_Read        (&MOS6502::SBC, A);    break;
      case 0xF5: ZeroPage_Read        (&MOS6502::SBC, A, X); break;

      // ---------------------------------------------------------------
      // STA / STX / STY
      // ---------------------------------------------------------------

      case 0x8D: Absolute_Write        (A);    break;
      case 0x9D: Absolute_Write        (A, X); break;
      case 0x99: Absolute_Write        (A, Y); break;
      case 0x81: IndexedIndirect_Write (A, X); break;
      case 0x91: IndirectIndexed_Write (A, Y); break;
      case 0x85: ZeroPage_Write        (A);    break;
      case 0x95: ZeroPage_Write        (A, X); break;

      case 0x8E: Absolute_Write        (X);    break;
      case 0x86: ZeroPage_Write        (X);    break;
      case 0x96: ZeroPage_Write        (X, Y); break;

      case 0x8C: Absolute_Write        (Y);    break;
      case 0x84: ZeroPage_Write        (Y);    break;
      case 0x94: ZeroPage_Write        (Y, X); break;

      // ---------------------------------------------------------------
      // Transfer Operations
      // ---------------------------------------------------------------

      case 0xAA: Idle(); X = Flags(A); break; // TAX
      case 0xA8: Idle(); Y = Flags(A); break; // TAY
      case 0xBA: Idle(); X = Flags(S); break; // TSX
      case 0x8A: Idle(); A = Flags(X); break; // TXA
      case 0x9A: Idle(); S = X;        break; // TXS (no flags)
      case 0x98: Idle(); A = Flags(Y); break; // TYA

      // ---------------------------------------------------------------
      // Illegal / Unknown
      // ---------------------------------------------------------------

      default:
        if (enableIllegal) {
          RunIllegal(opcode);
        } else {
          OnUnknownOpcode(opcode);
        }

        break;
    }
  }
}

//
// Addressing Modes
//

void MOS6502::Absolute_Modify(OPERATION operation) {
  AB.l = Fetch();
  AB.h = Fetch();
  BYTE input = Load(AB.w);
  Store(AB.w, input);
  BYTE output = 0;
  (this->*operation)(input, output);
  Store(AB.w, output);
}

void MOS6502::Absolute_Modify(OPERATION operation, BYTE index) {
  AB.l = Fetch();
  AB.h = Fetch();
  AB = IdleOnPageAlways(AB, index);
  BYTE input  = Load(AB.w);
  Store(AB.w, input);
  BYTE output = 0;
  (this->*operation)(input, output);
  Store(AB.w, output);
}

void MOS6502::Absolute_Read(OPERATION operation, BYTE &output) {
  AB.l = Fetch();
  AB.h = Fetch();
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::Absolute_Read(OPERATION operation, BYTE &output, BYTE index) {
  AB.l = Fetch();
  AB.h = Fetch();
  AB = IdleOnPageCrossed(AB, index);
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::Absolute_Write(BYTE &input) {
  AB.l = Fetch();
  AB.h = Fetch();
  Store(AB.w, input);
}

void MOS6502::Absolute_Write(BYTE &input, BYTE index) {
  AB.l = Fetch();
  AB.h = Fetch();
  AB = IdleOnPageAlways(AB, index);
  Store(AB.w, input);
}

void MOS6502::Immediate_Read(OPERATION operation, BYTE &output) {
  (this->*operation)(Fetch(), output);
}

void MOS6502::IndexedIndirect_Read(OPERATION operation, BYTE &output, BYTE index) {
  TB.w  = Fetch();
  Load(TB.w);
  TB.l += index;
  AB.l  = Load(TB.w);
  TB.l += 1;
  AB.h  = Load(TB.w);
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::IndexedIndirect_Write(BYTE &input, BYTE index) {
  TB.w  = Fetch();
  Load(TB.w);
  TB.l += index;
  AB.l  = Load(TB.w);
  TB.l += 1;
  AB.h  = Load(TB.w);
  Store(AB.w, input);
}

void MOS6502::IndirectIndexed_Read(OPERATION operation, BYTE &output, BYTE index) {
  TB.w  = Fetch();
  AB.l  = Load(TB.w);
  TB.l += 1;
  AB.h  = Load(TB.w);
  AB = IdleOnPageCrossed(AB, index);
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::IndirectIndexed_Write(BYTE &input, BYTE index) {
  TB.w  = Fetch();
  AB.l  = Load(TB.w);
  TB.l += 1;
  AB.h  = Load(TB.w);
  AB = IdleOnPageAlways(AB, index);
  Store(AB.w, input);
}

void MOS6502::ZeroPage_Modify(OPERATION operation) {
  AB.w = Fetch();
  BYTE input  = Load(AB.w);
  Store(AB.w, input);
  BYTE output = 0;
  (this->*operation)(input, output);
  Store(AB.w, output);
}

void MOS6502::ZeroPage_Modify(OPERATION operation, BYTE index) {
  AB.w  = Fetch();
  Load(AB.w);
  AB.l += index;
  BYTE input  = Load(AB.w);
  Store(AB.w, input);
  BYTE output = 0;
  (this->*operation)(input, output);
  Store(AB.w, output);
}

void MOS6502::ZeroPage_Read(OPERATION operation, BYTE &output) {
  AB.w = Fetch();
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::ZeroPage_Read(OPERATION operation, BYTE &output, BYTE index) {
  AB.w  = Fetch();
  Load(AB.w);
  AB.l += index;
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::ZeroPage_Write(BYTE &input) {
  AB.w = Fetch();
  Store(AB.w, input);
}

void MOS6502::ZeroPage_Write(BYTE &input, BYTE index) {
  AB.w  = Fetch();
  Load(AB.w);
  AB.l += index;
  Store(AB.w, input);
}

//
// Operations
//

void MOS6502::ADC(BYTE input, BYTE &output) {
  uint16_t temp = A + input + P.C;
  P.V = (~(A ^ input) & (A ^ temp) & 0x80) ? 1 : 0;

  if (P.D && enableBCD) {
    if ((temp & 0x00F) > 0x09) { temp += 0x06; }
    if ((temp & 0xFF0) > 0x90) { temp += 0x60; }
  }

  P.C = (temp & 0x0100) ? 1 : 0;
  output = Flags(temp & 0xFF);
}

void MOS6502::AND(BYTE input, BYTE &output) {
  output = Flags(A & input);
}

void MOS6502::ASL(BYTE input, BYTE &output) {
  P.C = (input & 0x80) ? 1 : 0;
  output = Flags(input << 1);
}

void MOS6502::BIT(BYTE input, BYTE & /*output*/) {
  P.N = (input & 0x80) ? 1 : 0;
  P.V = (input & 0x40) ? 1 : 0;
  P.Z = (A & input)    ? 0 : 1;
}

void MOS6502::CMP(BYTE input, BYTE &output) {
  P.C = (output >= input)         ? 1 : 0;
  P.N = ((output - input) & 0x80) ? 1 : 0;
  P.Z = (output == input)         ? 1 : 0;
}

void MOS6502::DEC(BYTE input, BYTE &output) {
  output = Flags(input - 1);
}

void MOS6502::EOR(BYTE input, BYTE &output) {
  output = Flags(A ^ input);
}

void MOS6502::INC(BYTE input, BYTE &output) {
  output = Flags(input + 1);
}

void MOS6502::LDx(BYTE input, BYTE &output) {
  output = Flags(input);
}

void MOS6502::LSR(BYTE input, BYTE &output) {
  P.C = (input & 0x01) ? 1 : 0;
  output = Flags(input >> 1);
}

void MOS6502::ORA(BYTE input, BYTE &output) {
  output = Flags(A | input);
}

void MOS6502::ROL(BYTE input, BYTE &output) {
  output = Flags((input << 1) | P.C);
  P.C = (input & 0x80) ? 1 : 0;
}

void MOS6502::ROR(BYTE input, BYTE &output) {
  output = Flags((input >> 1) | (P.C << 7));
  P.C = (input & 0x01) ? 1 : 0;
}

void MOS6502::SBC(BYTE input, BYTE &output) {
  ADC(~input, output);
}
