#include "arm7tdmi.h"

#define RD(op) ((op >> 16) & 0xF)
#define RN(op) ((op >> 12) & 0xF)
#define RS(op) ((op >> 8) & 0xF)
#define RM(op) (op & 0xF)
#define RDHI(op) RD(op)
#define RDLO(op) RN(op)


void Arm7tdmi::MUL(u32 op) {
	u32 result = (rReg(RM(op)) * rReg(RS(op))) & MASK_32BIT;
	wReg(RD(op), result);
	if (op & BIT(20)) {
		cpsr &= ~N & ~Z;
		cpsr |= (result & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
	}
}

void Arm7tdmi::MLA(u32 op) {
	u32 result = (rReg(RM(op)) * rReg(RS(op)) + rReg(RN(op))) & MASK_32BIT;
	wReg(RD(op), result);
	if (op & BIT(20)) {
		cpsr &= ~N & ~Z;
		cpsr |= (result & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
	}
}

void Arm7tdmi::UMULL(u32 op) {
	u64 result = ((u64)rReg(RM(op)) * (u64)rReg(RS(op)));
	u32 highPart = (u32)(result >> 32);
	wReg(RDLO(op), result & MASK_32BIT);
	wReg(RDHI(op), highPart);
	if (op & BIT(20)) {
		cpsr &= ~N & ~Z;
		cpsr |= (highPart & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
	}
}

void Arm7tdmi::UMLAL(u32 op) {
	u64 result = ((u64)rReg(RM(op)) * (u64)rReg(RS(op))) + ((u64)r[RDHI(op)] << 32) + r[RDLO(op)];
	u32 highPart = (result >> 32) & MASK_32BIT;
	wReg(RDLO(op), result & MASK_32BIT);
	wReg(RDHI(op), (result >> 32) & MASK_32BIT);
	if (op & BIT(20)) {
		cpsr &= ~N & ~Z;
		cpsr |= (highPart & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
	}
}

void Arm7tdmi::SMULL(u32 op) {
	s64 result = (s64)((s32)rReg(RM(op))) * (s64)((s32)rReg(RS(op)));
	u32 highPart = (result >> 32) & MASK_32BIT;
	wReg(RDLO(op), result & MASK_32BIT);
	wReg(RDHI(op), (result >> 32) & MASK_32BIT);
	if (op & BIT(20)) {
		cpsr &= ~N & ~Z;
		cpsr |= (highPart & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
	}
}

void Arm7tdmi::SMLAL(u32 op) {
	s64 result = (s64)((s32)rReg(RM(op))) * (s64)((s32)rReg(RS(op))) + ((s64)r[RDHI(op)] << 32) + r[RDLO(op)];
	u32 highPart = (result >> 32) & MASK_32BIT;
	wReg(RDLO(op), result & MASK_32BIT);
	wReg(RDHI(op), (result >> 32) & MASK_32BIT);
	if (op & BIT(20)) {
		cpsr &= ~N & ~Z;
		cpsr |= (highPart & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
	}
}

//TO BE OPTIMIZED!!
void Arm7tdmi::execMultiply(u32 op) {
	if (op & BIT(23)) { // Multiply long
		if (op & BIT(22)) { // Signed
			if (op & BIT(21)) { // Accumulate
				SMLAL(op);
			}
			else { // No accumulate
				SMULL(op);
			}
		}
		else { // Unsigned
			if (op & BIT(21)) { // Accumulate
				UMLAL(op);
			}
			else { // No accumulate
				UMULL(op);
			}
		}
	}
	else {
		if (op & BIT(21)) { // Accumulate
			MLA(op);
		}
		else { // No accumulate
			MUL(op);
		}
	}
}