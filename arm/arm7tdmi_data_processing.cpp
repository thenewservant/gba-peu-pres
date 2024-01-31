#include "arm7tdmi.h"

#define RD(op)	  ((op >> 12) & 0xF)
#define RN(op)	  ((op >> 16) & 0xF)

#define BIT_S(op) (op & BIT(20))

// Computes the immediate offset value, involving a ROR operation,
// and returns the carry out of this ROR operation.
u32 immOffset(u32 op, u32 cpsr, u8* carryOut) {
	u8 rotateImm = (op >> 8) & 0xF;
	u8 imm = op & 0xFF;

	u32 result = (imm >> (2 * rotateImm)) | (imm << (32 - 2 * rotateImm));
	*carryOut = (result) ? result & BIT(31) : cpsr & C;
	return result;
}

void Arm7tdmi::checkCPSR_DP(u32& op, const u8& carryOut) {
	if ((op & BIT_S(op)) && (RD(op) == 15)) {
		if (CURRENT_MODE_HAS_SPSR) {
			cpsr = getSPSRValue();
		}
	}
	else if ((op & BIT_S(op))) {
		cpsr &= ~N & ~Z & ~C;
		cpsr |= (rReg(RD(op)) & BIT(31)) ? N : 0;
		cpsr |= (rReg(RD(op)) == 0) ? Z : 0;
		cpsr |= carryOut ? C : 0;
	}
}


void Arm7tdmi::AND(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) & immOffset(op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::EOR(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) ^ immOffset(op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

// UNSAFE vvvv (missing V flag)

void Arm7tdmi::SUB(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) - immOffset(op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::RSB(u32 op) {
	u8 carryOut;
	wReg(RD(op), immOffset(op, cpsr, &carryOut) - rReg(RN(op)));
	checkCPSR_DP(op, carryOut);
}


void Arm7tdmi::ADD(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) + immOffset(op, cpsr, &carryOut));
	/*if S == 1 and Rd == R15 then
		if CurrentModeHasSPSR() then
			CPSR = SPSR
		else UNPREDICTABLE 
		
	else if S == 1 then
		N Flag = Rd[31]
		Z Flag = if Rd == 0 then 1 else 0
		C Flag = CarryFrom(Rn + shifter_operand)
		V Flag = OverflowFrom(Rn + shifter_operand)*/
	if (BIT_S(op) && (RD(op) == 15)) {
		if (CURRENT_MODE_HAS_SPSR) {
			cpsr = getSPSRValue();
		}
	}
	else if (BIT_S(op)) {
		cpsr &= ~N & ~Z & ~C & ~V;
		cpsr |= (rReg(RD(op)) & BIT(31)) ? N : 0;
		cpsr |= (rReg(RD(op)) == 0) ? Z : 0;
		cpsr |= carryOut ? C : 0;

		//UNTESTED : V FLAG CALCULATION
		cpsr |= ((rReg(RN(op)) ^ immOffset(op, cpsr, &carryOut)) & (rReg(RD(op)) ^ rReg(RN(op))) & BIT(31)) ? V : 0;

	}
}

void Arm7tdmi::ADC(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) + immOffset(op, cpsr, &carryOut) + (cpsr & C));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::SBC(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) - immOffset(op, cpsr, &carryOut) - (cpsr & C));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::RSC(u32 op) {
	u8 carryOut;
	wReg(RD(op), immOffset(op, cpsr, &carryOut) - rReg(RN(op)) - (cpsr & C));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::CMP(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) - immOffset(op, cpsr, &carryOut);
	cpsr &= ~N & ~Z & ~C;
	cpsr |= (result & BIT(31)) ? N : 0;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= carryOut ? C : 0;
}

void Arm7tdmi::CMN(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) + immOffset(op, cpsr, &carryOut);
	cpsr &= ~N & ~Z & ~C;
	cpsr |= (result & BIT(31)) ? N : 0;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= carryOut ? C : 0;
}

// UNSAFE ^^^^


void Arm7tdmi::TST(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) & immOffset(op, cpsr, &carryOut);
	cpsr &= ~N & ~Z & ~C;
	cpsr |= (result & BIT(31)) ? N : 0;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= carryOut ? C : 0;
}

void Arm7tdmi::TEQ(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) ^ immOffset(op, cpsr, &carryOut);
	cpsr &= ~N & ~Z & ~C;
	cpsr |= (result & BIT(31)) ? N : 0;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= carryOut ? C : 0;
}

void Arm7tdmi::ORR(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) | immOffset(op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::MOV(u32 op) {
	u8 carryOut;
	wReg(RD(op), immOffset(op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::BIC(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) & ~immOffset(op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::MVN(u32 op) {
	u8 carryOut;
	wReg(RD(op), ~immOffset(op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::MSR_IMM(u32 op) {

}

void Arm7tdmi::MSR_REG(u32 op) {

}

void Arm7tdmi::MRS(u32 op) {

}