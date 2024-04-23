#include "arm7tdmi.h"

#define L_BIT_SET(op) (BIT(24) & op)
#define RM(op) (op & 0xF)
void Arm7tdmi::B_BL(u32 op) {

    if (L_BIT_SET(op)) {
        wReg(14, r[15] + 4); // address of the instruction after the branch instruction
    }

    s32 offset = (s32)((op & 0xFFFFFF) | ((op & BIT(23)) ? 0x3F000000 : 0)); // Sign extend the offset
    //printf("offset: %d\n", offset);
    wReg(15, r[15] + offset * 4 + 8);
}

void Arm7tdmi::BX(u32 op) {
    
    u32 destRegVal = rReg(RM(op));
    printf("BX CALLED: %d\n", destRegVal & 0x1);
    cpsr &= ~T;
    cpsr |= (destRegVal & 0x1) ? T : 0; // Set T bit to bit 0 of Rm
    r[15] = (destRegVal & 0xFFFFFFFE); // Clear the bottom two bits of the address
}