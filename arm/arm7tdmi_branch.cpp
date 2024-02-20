#include "arm7tdmi.h"

#define L_BIT_SET(op) (BIT(24) & op)
#define RM(op) (op & 0xF)
void Arm7tdmi::B_BL(u32 op) {

    if (L_BIT_SET(op)) {
        wReg(14, r[15]); // address of the instruction after the branch instruction
    }

    s32 offset = (s32)((op & 0xFFFFFF) | ((op & BIT(23)) ? 0x3F000000 : 0)); // Sign extend the offset
    r[15] = r[15] + offset * 4+4;
}

void Arm7tdmi::BX(u32 op) {
    cpsr &= ~T;
    cpsr |= (rReg(RM(op)) & 0x1) ? T : 0; // Set T bit to bit 0 of Rm
    if((rReg(RM(op)) & 0x1))printf("BX: T bit set\n");
    r[15] = (rReg(RM(op)) & 0xFFFFFFFE) + ((cpsr & T) ? 0 : 4); // Clear the bottom two bits of the address
}