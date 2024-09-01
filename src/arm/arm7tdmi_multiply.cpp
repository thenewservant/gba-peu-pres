#include "arm/arm7tdmi.h"

#define RD(op) ((op >> 16) & 0xF)
#define RN(op) ((op >> 12) & 0xF)
#define RS(op) ((op >> 8) & 0xF)
#define RM(op) (op & 0xF)
#define RDHI(op) RD(op)
#define RDLO(op) RN(op)

void Arm7tdmi::MULT_SHORT(u32 op) {
	u32 result = (rReg(RM(op)) * rReg(RS(op))) ;
	if (op & BIT(21)) {//Accumulate
		result += rReg(RN(op));
	}
	wReg(RD(op), result);
	if (op & BIT(20)) {
		cpsr &= ~N & ~Z;
		cpsr |= (result & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
	}
}

void Arm7tdmi::MULT_LONG(u32 op) {
	u64 result;
	if (op & BIT(22)) { // Signed
		result = (s64)((s32)rReg(RM(op))) * (s64)((s32)rReg(RS(op)));
	}
	else { // Unsigned
		result = (u64)rReg(RM(op)) * (u64)rReg(RS(op));
	}
	if (op & BIT(21)) {//Accumulate
		result += ((u64)rReg(RDHI(op)) << 32) + rReg(RDLO(op));
	}
	u32 highPart = (u32)(result >> 32);
	wReg(RDLO(op), (u32)result);
	wReg(RDHI(op), highPart);
	if (op & BIT(20)) {
		cpsr &= ~N & ~Z;
		cpsr |= (highPart & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
	}
}

void Arm7tdmi::execMultiply(u32 op) {
	if (op & BIT(23)) {
		MULT_LONG(op);
	}
	else {
		MULT_SHORT(op);
	}
}
