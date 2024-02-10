#include "arm7tdmi.h"

#define L_BIT_SET(op) (BIT(24) & op)

void Arm7tdmi::B_BL(u32 op) {
    
    if (L_BIT_SET(op)) {
        wReg(14, r[15]); // address of the instruction after the branch instruction
    }

	s32 offset = (s32)((op & 0xFFFFFF) | ((op & BIT(23))?0x3F000000:0)); // Sign extend the offset
	r[15] = r[15] + offset*4 +4;

   
}

void Arm7tdmi::BX(u32 op) {
    cpsr &= ~T;
    cpsr |= (op & 0x1) ? T : 0; // Set T bit to bit 0 of Rm
}