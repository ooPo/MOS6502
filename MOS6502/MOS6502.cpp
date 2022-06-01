//
// MOS6502.cpp
// by Naomi Peori <naomi@peori.ca>
//

#include <stdio.h>
#include "MOS6502.h"

void MOS6502::Run(void) {
  running = true;

  while(running) {

    if (signals[INTERRUPT::NMI]) {
      Interrupt(0xFFFA);
      signals[INTERRUPT::NMI] = false;
    }
    else if (! P.I) {
      if (signals[INTERRUPT::IRQ]) {
        Interrupt(0xFFFE);
        signals[INTERRUPT::IRQ] = false;
      }
    }

    uint8_t opcode = Fetch();
    // printf("%04X  %02X A:%02X X:%02X Y:%02X P:%02X SP:%02X\n", PC - 1, opcode, A, X, Y, P.value, S);

    switch(opcode) {
      // ADC - Add with Carry
      case 0x6D: Absolute_Read        (&MOS6502::ADC, A);    break;
      case 0x7D: Absolute_Read        (&MOS6502::ADC, A, X); break;
      case 0x79: Absolute_Read        (&MOS6502::ADC, A, Y); break;
      case 0x69: Immediate_Read       (&MOS6502::ADC, A);    break;
      case 0x61: IndexedIndirect_Read (&MOS6502::ADC, A, X); break;
      case 0x71: IndirectIndexed_Read (&MOS6502::ADC, A, Y); break;
      case 0x65: ZeroPage_Read        (&MOS6502::ADC, A);    break;
      case 0x75: ZeroPage_Read        (&MOS6502::ADC, A, X); break;

      // AND - Logical AND
      case 0x2D: Absolute_Read        (&MOS6502::AND, A);    break;
      case 0x3D: Absolute_Read        (&MOS6502::AND, A, X); break;
      case 0x39: Absolute_Read        (&MOS6502::AND, A, Y); break;
      case 0x29: Immediate_Read       (&MOS6502::AND, A);    break;
      case 0x21: IndexedIndirect_Read (&MOS6502::AND, A, X); break;
      case 0x31: IndirectIndexed_Read (&MOS6502::AND, A, Y); break;
      case 0x25: ZeroPage_Read        (&MOS6502::AND, A);    break;
      case 0x35: ZeroPage_Read        (&MOS6502::AND, A, X); break;

      // ASL - Arithmetic Shift Left
      case 0x0E: Absolute_Modify      (&MOS6502::ASL);       break;
      case 0x1E: Absolute_Modify      (&MOS6502::ASL, X);    break;
      case 0x06: ZeroPage_Modify      (&MOS6502::ASL);       break;
      case 0x16: ZeroPage_Modify      (&MOS6502::ASL, X);    break;
      case 0x0A: ASL(A,A); break;

      // BCC - Branch if Carry Clear
      case 0x90: Branch(! P.C); break;

      // BCS - Branch if Carry Set
      case 0xB0: Branch(P.C); break;

      // BEQ - Branch if Equal
      case 0xF0: Branch(P.Z == 1); break;

      // BIT - Bit Test
      case 0x2C: Absolute_Read        (&MOS6502::BIT, A);    break;
      case 0x24: ZeroPage_Read        (&MOS6502::BIT, A);    break;

      // BMI - Branch if Minus
      case 0x30: Branch(P.N); break;

      // BNE - Branch if Not Equal
      case 0xD0: Branch(P.Z == 0); break;

      // BPL - Branch if Positive
      case 0x10: Branch(! P.N); break;

      // BRK - Force Interrupt
      case 0x00:
        Fetch();
        Push(PC.h);
        Push(PC.l);
        Push(P.value | 0x30);
        PC.l = Load(0xFFFE);
        PC.h = Load(0xFFFF);
        P.I = 1;
        break;

      // BVC - Branch if Overflow Clear
      case 0x50: Branch(! P.V); break;

      // BVS - Branch if Overflow Set
      case 0x70: Branch(P.V); break;

      // CLC - Clear Carry Flag
      case 0x18: Idle(); P.C = false; break;

      // CLD - Clear Decimal Mode
      case 0xD8: Idle(); P.D = false; break;

      // CLI - Clear Interrupt Disable
      case 0x58: Idle(); P.I = false; break;

      // CLV - Clear Overflow Flag
      case 0xB8: Idle(); P.V = false; break;

      // CMP - Compare
      case 0xCD: Absolute_Read        (&MOS6502::CMP, A);    break;
      case 0xDD: Absolute_Read        (&MOS6502::CMP, A, X); break;
      case 0xD9: Absolute_Read        (&MOS6502::CMP, A, Y); break;
      case 0xC9: Immediate_Read       (&MOS6502::CMP, A);    break;
      case 0xC1: IndexedIndirect_Read (&MOS6502::CMP, A, X); break;
      case 0xD1: IndirectIndexed_Read (&MOS6502::CMP, A, Y); break;
      case 0xC5: ZeroPage_Read        (&MOS6502::CMP, A);    break;
      case 0xD5: ZeroPage_Read        (&MOS6502::CMP, A, X); break;

      // CPX - Compare X Register
      case 0xEC: Absolute_Read        (&MOS6502::CMP, X);    break;
      case 0xE0: Immediate_Read       (&MOS6502::CMP, X);    break;
      case 0xE4: ZeroPage_Read        (&MOS6502::CMP, X);    break;

      // CPY - Compare Y Register
      case 0xCC: Absolute_Read        (&MOS6502::CMP, Y);    break;
      case 0xC0: Immediate_Read       (&MOS6502::CMP, Y);    break;
      case 0xC4: ZeroPage_Read        (&MOS6502::CMP, Y);    break;

      // DEC - Decrement Memory
      case 0xCE: Absolute_Modify      (&MOS6502::DEC);       break;
      case 0xDE: Absolute_Modify      (&MOS6502::DEC, X);    break;
      case 0xC6: ZeroPage_Modify      (&MOS6502::DEC);       break;
      case 0xD6: ZeroPage_Modify      (&MOS6502::DEC, X);    break;

      // DEX - Decrement X Register
      case 0xCA: X = Flags(X - 1); break;

      // DEY - Decrement Y Register
      case 0x88: Y = Flags(Y - 1); break;

      // EOR - Exclusive OR
      case 0x4D: Absolute_Read        (&MOS6502::EOR, A);    break;
      case 0x5D: Absolute_Read        (&MOS6502::EOR, A, X); break;
      case 0x59: Absolute_Read        (&MOS6502::EOR, A, Y); break;
      case 0x49: Immediate_Read       (&MOS6502::EOR, A);    break;
      case 0x41: IndexedIndirect_Read (&MOS6502::EOR, A, X); break;
      case 0x51: IndirectIndexed_Read (&MOS6502::EOR, A, Y); break;
      case 0x45: ZeroPage_Read        (&MOS6502::EOR, A);    break;
      case 0x55: ZeroPage_Read        (&MOS6502::EOR, A, X); break;

      // INC - Increment Memory
      case 0xEE: Absolute_Modify      (&MOS6502::INC);       break;
      case 0xFE: Absolute_Modify      (&MOS6502::INC, X);    break;
      case 0xE6: ZeroPage_Modify      (&MOS6502::INC);       break;
      case 0xF6: ZeroPage_Modify      (&MOS6502::INC, X);    break;

      // INX - Increment X Register
      case 0xE8: X = Flags(X + 1); break;

      // INY - Increment Y Register
      case 0xC8: Y = Flags(Y + 1); break;

      // JMP - Jump (ABS)
      case 0x4C:
        AB.l = Fetch();
        AB.h = Fetch();
        PC = AB;
        break;

      // JMP - Jump (IABS)
      case 0x6C:
        AB.l = Fetch();
        AB.h = Fetch();
        PC.l = Load(AB.w);
        AB.l += 1;
        PC.h = Load(AB.w);
        break;

      // JSR - Jump to Subroutine
      case 0x20:
        AB.l = Fetch();
        Push(PC.h);
        Push(PC.l);
        AB.h = Fetch();
        PC = AB;
        break;

      // LDA - Load Accumulator
      case 0xAD: Absolute_Read        (&MOS6502::LDx, A);    break;
      case 0xBD: Absolute_Read        (&MOS6502::LDx, A, X); break;
      case 0xB9: Absolute_Read        (&MOS6502::LDx, A, Y); break;
      case 0xA9: Immediate_Read       (&MOS6502::LDx, A);    break;
      case 0xA1: IndexedIndirect_Read (&MOS6502::LDx, A, X); break;
      case 0xB1: IndirectIndexed_Read (&MOS6502::LDx, A, Y); break;
      case 0xA5: ZeroPage_Read        (&MOS6502::LDx, A);    break;
      case 0xB5: ZeroPage_Read        (&MOS6502::LDx, A, X); break;

      // LDX - Load X Register
      case 0xAE: Absolute_Read        (&MOS6502::LDx, X);    break;
      case 0xBE: Absolute_Read        (&MOS6502::LDx, X, Y); break;
      case 0xA2: Immediate_Read       (&MOS6502::LDx, X);    break;
      case 0xA6: ZeroPage_Read        (&MOS6502::LDx, X);    break;
      case 0xB6: ZeroPage_Read        (&MOS6502::LDx, X, Y); break;

      // LDY - Load Y Register
      case 0xAC: Absolute_Read        (&MOS6502::LDx, Y);    break;
      case 0xBC: Absolute_Read        (&MOS6502::LDx, Y, X); break;
      case 0xA0: Immediate_Read       (&MOS6502::LDx, Y);    break;
      case 0xA4: ZeroPage_Read        (&MOS6502::LDx, Y);    break;
      case 0xB4: ZeroPage_Read        (&MOS6502::LDx, Y, X); break;

      // LSR - Logical Shift Right
      case 0x4E: Absolute_Modify      (&MOS6502::LSR);       break;
      case 0x5E: Absolute_Modify      (&MOS6502::LSR, X);    break;
      case 0x46: ZeroPage_Modify      (&MOS6502::LSR);       break;
      case 0x56: ZeroPage_Modify      (&MOS6502::LSR, X);    break;
      case 0x4A: LSR(A,A); break;

      // NOP - No Operation
      case 0xEA: Idle(); break;

      // ORA - Logical Inclusive OR
      case 0x0D: Absolute_Read        (&MOS6502::ORA, A);    break;
      case 0x1D: Absolute_Read        (&MOS6502::ORA, A, X); break;
      case 0x19: Absolute_Read        (&MOS6502::ORA, A, Y); break;
      case 0x09: Immediate_Read       (&MOS6502::ORA, A);    break;
      case 0x01: IndexedIndirect_Read (&MOS6502::ORA, A, X); break;
      case 0x11: IndirectIndexed_Read (&MOS6502::ORA, A, Y); break;
      case 0x05: ZeroPage_Read        (&MOS6502::ORA, A);    break;
      case 0x15: ZeroPage_Read        (&MOS6502::ORA, A, X); break;

      // PHA - Push Accumulator
      case 0x48: Push(A); break;

      // PHP - Push Processor Status
      case 0x08: Push(P.value | 0x30); break;

      // PLA - Pull Accumulator
      case 0x68: A = Flags(Pull()); break;

      // PLP - Pull Processor Status
      case 0x28:
        P.value = Pull();
        P.value &= ~0x10;
        P.value |=  0x20;
        break;

      // ROL - Rotate Left
      case 0x2E: Absolute_Modify      (&MOS6502::ROL);       break;
      case 0x3E: Absolute_Modify      (&MOS6502::ROL, X);    break;
      case 0x26: ZeroPage_Modify      (&MOS6502::ROL);       break;
      case 0x36: ZeroPage_Modify      (&MOS6502::ROL, X);    break;
      case 0x2A: ROL(A,A); break;

      // ROR - Rotate Right
      case 0x6E: Absolute_Modify      (&MOS6502::ROR);       break;
      case 0x7E: Absolute_Modify      (&MOS6502::ROR, X);    break;
      case 0x66: ZeroPage_Modify      (&MOS6502::ROR);       break;
      case 0x76: ZeroPage_Modify      (&MOS6502::ROR, X);    break;
      case 0x6A: ROR(A,A); break;

      // RTI - Return from Interrupt
      case 0x40:
        P.value = Pull();
        P.value &= ~0x10;
        P.value |=  0x20;
        PC.l = Pull();
        PC.h = Pull();
        break;

      // RTS - Return from Subroutine
      case 0x60:
        Idle();
        Load(0x0100 | S);
        PC.l = Pull();
        PC.h = Pull();
        Fetch();
        break;

      // SBC - Subtract with Carry
      case 0xED: Absolute_Read        (&MOS6502::SBC, A);    break;
      case 0xFD: Absolute_Read        (&MOS6502::SBC, A, X); break;
      case 0xF9: Absolute_Read        (&MOS6502::SBC, A, Y); break;
      case 0xE9: Immediate_Read       (&MOS6502::SBC, A);    break;
      case 0xE1: IndexedIndirect_Read (&MOS6502::SBC, A, X); break;
      case 0xF1: IndirectIndexed_Read (&MOS6502::SBC, A, Y); break;
      case 0xE5: ZeroPage_Read        (&MOS6502::SBC, A);    break;
      case 0xF5: ZeroPage_Read        (&MOS6502::SBC, A, X); break;

      // SEC - Set Carry Flag
      case 0x38: Idle(); P.C = true; break;

      // SED - Set Decimal Flag
      case 0xF8: Idle(); P.D = true; break;

      // SEI - Set Interrupt Disable
      case 0x78: Idle(); P.I = true; break;

      // STA - Store Accumulator
      case 0x8D: Absolute_Write        (A);    break;
      case 0x9D: Absolute_Write        (A, X); break;
      case 0x99: Absolute_Write        (A, Y); break;
      case 0x81: IndexedIndirect_Write (A, X); break;
      case 0x91: IndirectIndexed_Write (A, Y); break;
      case 0x85: ZeroPage_Write        (A);    break;
      case 0x95: ZeroPage_Write        (A, X); break;

      // STX - Store X Register
      case 0x8E: Absolute_Write        (X);    break;
      case 0x86: ZeroPage_Write        (X);    break;
      case 0x96: ZeroPage_Write        (X, Y); break;

      // STY - Store Y Register
      case 0x8C: Absolute_Write        (Y);    break;
      case 0x84: ZeroPage_Write        (Y);    break;
      case 0x94: ZeroPage_Write        (Y, X); break;

      // TAX - Transfer Accumulator to X
      case 0xAA: X = Flags(A); break;

      // TAY - Transfer Accumulator to Y
      case 0xA8: Y = Flags(A); break;

      // TSX - Transfer Stack Pointer to X
      case 0xBA: X = Flags(S); break;

      // TXA - Transfer X to Accumulator
      case 0x8A: A = Flags(X); break;

      // TXS - Transfer X to Stack Pointer
      case 0x9A: S = X; break;

      // TYA - Transfer Y to Accumulator
      case 0x98: A = Flags(Y); break;

      default:
        printf("UNKNOWN OPCODE %02X\n", opcode);
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
  Store(AB.w, input); // dummy write
  BYTE output = 0;
  (this->*operation)(input, output);
  Store(AB.w, output);
}

void MOS6502::Absolute_Modify(OPERATION operation, BYTE index) {
  AB.l = Fetch();
  AB.h = Fetch();
  AB = IdleOnPageAlways(AB, index);
  BYTE input = Load(AB.w);
  Store(AB.w, input); // dummy write
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
  TB.w = Fetch();
  Load(TB.w); // dummy read
  TB.l += index;
  AB.l = Load(TB.w);
  TB.l += 1;
  AB.h = Load(TB.w);
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::IndexedIndirect_Write(BYTE &input, BYTE index) {
  TB.w = Fetch();
  Load(TB.w); // dummy read
  TB.l += index;
  AB.l = Load(TB.w);
  TB.l += 1;
  AB.h = Load(TB.w);
  Store(AB.w, input);
}

void MOS6502::IndirectIndexed_Read(OPERATION operation, BYTE &output, BYTE index) {
  TB.w = Fetch();
  AB.l = Load(TB.w);
  TB.l += 1;
  AB.h = Load(TB.w);
  AB = IdleOnPageCrossed(AB, index);
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::IndirectIndexed_Write(BYTE &input, BYTE index) {
  TB.w = Fetch();
  AB.l = Load(TB.w);
  TB.l += 1;
  AB.h = Load(TB.w);
  AB = IdleOnPageAlways(AB, index);
  Store(AB.w, input);
}

void MOS6502::ZeroPage_Modify(OPERATION operation) {
  AB.w = Fetch();
  BYTE data = Load(AB.w);
  Store(AB.w, data); // dummy write
  BYTE output = 0;
  (this->*operation)(data, output);
  Store(AB.w, output);
}

void MOS6502::ZeroPage_Modify(OPERATION operation, BYTE index) {
  AB.w = Fetch();
  Load(AB.w); // dummy read
  AB.l += index;
  BYTE data = Load(AB.w);
  Store(AB.w, data); // dummy write
  BYTE output = 0;
  (this->*operation)(data, output);
  Store(AB.w, output);
}

void MOS6502::ZeroPage_Read(OPERATION operation, BYTE &output) {
  AB.w = Fetch();
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::ZeroPage_Read(OPERATION operation, BYTE &output, BYTE index) {
  AB.w = Fetch();
  Load(AB.w); // dummy read
  AB.l += index;
  (this->*operation)(Load(AB.w), output);
}

void MOS6502::ZeroPage_Write(BYTE &input) {
  AB.w = Fetch();
  Store(AB.w, input);
}

void MOS6502::ZeroPage_Write(BYTE &input, BYTE index) {
  AB.w = Fetch();
  Load(AB.w); // dummy read
  AB.l += index;
  Store(AB.w, input);
}

//
// Operations
//

void MOS6502::ADC(const BYTE input, BYTE &output) {
  if (! P.D || ! enableBCD) {
    uint16_t temp = A + input + P.C;
    P.V = (~(A ^ input) & (A ^ temp) & 0x80) ? 1 : 0;
    P.C = (temp & 0x0100) ? 1 : 0;
    output = Flags(temp);
  }
  else {
    Idle(); // TODO: BCD mode.
  }
}

void MOS6502::AND(const BYTE input, BYTE &output) {
  output = Flags(A & input);
}

void MOS6502::ASL(const BYTE input, BYTE &output) {
  output = Flags(input << 1);
  P.C = (input & 0x80) ? 1 : 0;
}

void MOS6502::BIT(const BYTE input, BYTE &output) {
  uint8_t temp = A & input;
  P.N = (input & 0x80) ? 1 : 0;
  P.V = (input & 0x40) ? 1 : 0;
  P.Z = (temp  & 0xFF) ? 0 : 1;
}

void MOS6502::CMP(const BYTE input, BYTE &output) {
  uint16_t temp = output - input;
  P.C = (temp & 0x0100) ? 0 : 1;
  P.N = (temp & 0x0080) ? 1 : 0;
  P.Z = (temp & 0x00FF) ? 0 : 1;
}

void MOS6502::DEC(const BYTE input, BYTE &output) {
  output = Flags(input - 1);
}

void MOS6502::EOR(const BYTE input, BYTE &output) {
  output = Flags(A ^ input);
}

void MOS6502::INC(const BYTE input, BYTE &output) {
  output = Flags(input + 1);
}

void MOS6502::LDx(const BYTE input, BYTE &output) {
  output = Flags(input);
}

void MOS6502::LSR(const BYTE input, BYTE &output) {
  output = Flags(input >> 1);
  P.C = (input & 0x01) ? 1 : 0;
}

void MOS6502::ORA(const BYTE input, BYTE &output) {
  output = Flags(A | input);
}

void MOS6502::ROL(const BYTE input, BYTE &output) {
  output = Flags((input << 1) | (P.C ? 0x01 : 0x00));
  P.C = (input & 0x80) ? 1 : 0;
}

void MOS6502::ROR(const BYTE input, BYTE &output) {
  output = Flags((P.C ? 0x80 : 0x00) | (input >> 1));
  P.C = (input & 0x01) ? 1 : 0;
}

void MOS6502::SBC(const BYTE input, BYTE &output) {
  ADC(~input, output);
}
