#include "arm7tdmi.h"

#define L_BIT_SET(op) (BIT(24) & op)

void Arm7tdmi::B_BL(u32 op) {
    if (L_BIT_SET(op)) {
        wReg(14, r[15] + 4); // address of the instruction after the branch instruction
    }
    u32 offset = (op & 0xFFFFFF) << 2; // PC = PC + (SignExtend_30(signed_immed_24) << 2)
    r[15] += offset;
}

void Arm7tdmi::BX(u32 op) {
    if (evalCondition(cpsr, op)) {
        cpsr &= ~T;
        cpsr |= (op & 0x1) ? T : 0; // Set T bit to bit 0 of Rm
    }
}