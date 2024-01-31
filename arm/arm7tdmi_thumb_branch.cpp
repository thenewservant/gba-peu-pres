#include "arm7tdmi.h"

#define GET_BRANCH_OPCODE(op) ((op >> 8) & 0xF)
#define TEST_BRANCH(cond) if(cond){wReg(15, rReg(15) + (s32)(sOffSet << 1));}

enum THUMB_BRANCH {
    TB_BEQ = 0x0, // Z set
    TB_BNE = 0x1, // Z clear
    TB_BCS = 0x2, // C set
    TB_BCC = 0x3, // C clear
    TB_BMI = 0x4, // N set
    TB_BPL = 0x5, // N clear
    TB_BVS = 0x6, // V set
    TB_BVC = 0x7, // V clear
    TB_BHI = 0x8, // C set and Z clear
    TB_BLS = 0x9, // C clear or Z set
    TB_BGE = 0xA, // N equals V
    TB_BLT = 0xB, // N not equal to V
    TB_BGT = 0xC, // Z clear, and either N equals V, or N set and V clear
    TB_BLE = 0xD, // Z set, or N != V
};

void Arm7tdmi::TB_COND_BRANCH(u16 op) {
    s32 sOffSet = (s32)(op & 0xff);
    switch (GET_BRANCH_OPCODE(op)) {
    case TB_BEQ:
        TEST_BRANCH(cpsr & Z); break;
    case TB_BNE:
        TEST_BRANCH(!(cpsr & Z)); break;
    case TB_BCS:
        TEST_BRANCH(cpsr & C); break;
    case TB_BCC:
        TEST_BRANCH(!(cpsr & C)); break;
    case TB_BMI:
        TEST_BRANCH(cpsr & N); break;
    case TB_BPL:
        TEST_BRANCH(!(cpsr & N)); break;
    case TB_BVS:
        TEST_BRANCH(cpsr & V); break;
    case TB_BVC:
        TEST_BRANCH(!(cpsr & V)); break;
    case TB_BHI:
        TEST_BRANCH((cpsr & C) && !(cpsr & Z)); break;
    case TB_BLS:
        TEST_BRANCH(!(cpsr & C) || (cpsr & Z)); break;
    case TB_BGE:
        TEST_BRANCH((cpsr & N) == (cpsr & V)); break;
    case TB_BLT:
        TEST_BRANCH((cpsr & N) != (cpsr & V)); break;
    case TB_BGT:
        TEST_BRANCH(!(cpsr & Z) && ((cpsr & N) == (cpsr & V))); break;
    case TB_BLE:
        TEST_BRANCH((cpsr & Z) || ((cpsr & N) != (cpsr & V))); break;
    default:
        break;
    }
}

void Arm7tdmi::TB_UNCOND_BRANCH(u16 op) {
	s32 sOffSet =(s32)( op & 0x7ff);
    TEST_BRANCH(true);
}