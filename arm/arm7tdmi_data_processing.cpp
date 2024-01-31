#include "arm7tdmi.h"

#define RD(op)	  ((op >> 12) & 0xF)
#define RN(op)	  ((op >> 16) & 0xF)

#define BIT_S(op) (op & BIT(20))
#define BIT_I(op) (op & BIT(25))


#define CHECKCPSR_WITH_V_FLAG if (BIT_S(op) && (RD(op) == 15)) {\
								if (CURRENT_MODE_HAS_SPSR) {\
									cpsr = getSPSRValue();\
								}\
							}\
							else if (BIT_S(op)) {\
								cpsr &= ~N & ~Z & ~C & ~V;\
								cpsr |= (rReg(RD(op)) & BIT(31)) ? N : 0;\
								cpsr |= (rReg(RD(op)) == 0) ? Z : 0;\
								cpsr |= carryOut ? C : 0;\
								cpsr |= ((rReg(RN(op)) ^ operand2(this, op, cpsr, &carryOut)) & (rReg(RD(op)) ^ rReg(RN(op))) & BIT(31)) ? V : 0;\
							}

enum ALU_SHIFT_TYPE {
	LSL = 0,
	LSR = 1,
	ASR = 2,
	ROR = 3
};

// Computes the immediate offset value, involving a ROR operation,
// and returns the carry out of this ROR operation.
u32 operand2(Arm7tdmi* cpu, u32 op, u32 cpsr, u8* carryOut) {
	if (BIT_I(op)) {
		u8 rotateImm = (op >> 8) & 0xF;
		u8 imm = op & 0xFF;

		u32 result = (imm >> (2 * rotateImm)) | (imm << (32 - 2 * rotateImm));
		*carryOut = (result) ? result & BIT(31) : cpsr & C;
		return result;
	}
	else { // Register as 2nd operand
		u8 shiftType = (op >> 5) & 0x3;
		u8 shiftAmount = (op >> 7) & 0x1F;
		u8 rm = op & 0xF;

		u32 result = cpu->rReg(rm);
		*carryOut = (result) ? result & BIT(31) : cpsr & C;

		switch (shiftType) {
		case LSL:
			if (shiftAmount == 0) {
				*carryOut = cpsr & C;
				return result;
			}
			else {
				*carryOut = (result & BIT(32 - shiftAmount)) ? 1 : 0;
				return result << shiftAmount;
			}
		case LSR:
			if (shiftAmount == 0) {
				*carryOut = (result & BIT(31)) ? 1 : 0;
				return 0;
			}
			else {
				*carryOut = (result & BIT(shiftAmount - 1)) ? 1 : 0;
				return result >> shiftAmount;
			}
		case ASR:
			if (shiftAmount == 0) {
				*carryOut = (result & BIT(31)) ? 1 : 0;
				return (result & BIT(31)) ? 0xFFFFFFFF : 0;
			}
			else {
				*carryOut = (result & BIT(shiftAmount - 1)) ? 1 : 0;
				return (result & BIT(31)) ? (result >> shiftAmount) | (0xFFFFFFFF << (32 - shiftAmount)) : result >> shiftAmount;
			}
		case ROR:
			if (shiftAmount == 0) {
				*carryOut = (result & BIT(31)) ? 1 : 0;
				return result;
			}
			else {
				*carryOut = (result & BIT(shiftAmount - 1)) ? 1 : 0;
				return (result >> shiftAmount) | (result << (32 - shiftAmount));
			}
		}
	}
	
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
	wReg(RD(op), rReg(RN(op)) & operand2(this, op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::EOR(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) ^ operand2(this, op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

// UNSAFE vvvv (missing V flag)

void Arm7tdmi::SUB(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) - operand2(this, op, cpsr, &carryOut));
	CHECKCPSR_WITH_V_FLAG;
}

void Arm7tdmi::RSB(u32 op) {
	u8 carryOut;
	wReg(RD(op), operand2(this, op, cpsr, &carryOut) - rReg(RN(op)));
	CHECKCPSR_WITH_V_FLAG;
}


void Arm7tdmi::ADD(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) + operand2(this, op, cpsr, &carryOut));
	CHECKCPSR_WITH_V_FLAG;
}

void Arm7tdmi::ADC(u32 op) {
	u8 carryOut;
	u64 result = rReg(RN(op)) + operand2(this, op, cpsr, &carryOut) + ((cpsr & C) >0);
	wReg(RD(op), (u32)result);
	CHECKCPSR_WITH_V_FLAG;
}

void Arm7tdmi::SBC(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) - operand2(this, op, cpsr, &carryOut) - (cpsr & C));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::RSC(u32 op) {
	u8 carryOut;
	wReg(RD(op), operand2(this, op, cpsr, &carryOut) - rReg(RN(op)) - (cpsr & C));
	CHECKCPSR_WITH_V_FLAG;
}

void Arm7tdmi::CMP(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) - operand2(this, op, cpsr, &carryOut);
	CHECKCPSR_WITH_V_FLAG;

}

void Arm7tdmi::CMN(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) + operand2(this, op, cpsr, &carryOut);
	cpsr &= ~N & ~Z & ~C;
	cpsr |= (result & BIT(31)) ? N : 0;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= carryOut ? C : 0;
}

// UNSAFE ^^^^


void Arm7tdmi::TST(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) & operand2(this, op, cpsr, &carryOut);
	cpsr &= ~N & ~Z & ~C;
	cpsr |= (result & BIT(31)) ? N : 0;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= carryOut ? C : 0;
}

void Arm7tdmi::TEQ(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) ^ operand2(this, op, cpsr, &carryOut);
	cpsr &= ~N & ~Z & ~C;
	cpsr |= (result & BIT(31)) ? N : 0;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= carryOut ? C : 0;
}

void Arm7tdmi::ORR(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) | operand2(this, op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::MOV(u32 op) {
	u8 carryOut;
	wReg(RD(op), operand2(this, op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::BIC(u32 op) {
	u8 carryOut;
	wReg(RD(op), rReg(RN(op)) & ~operand2(this, op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::MVN(u32 op) {
	u8 carryOut;
	wReg(RD(op), ~operand2(this, op, cpsr, &carryOut));
	checkCPSR_DP(op, carryOut);
}

void Arm7tdmi::MSR_IMM(u32 op) {

}

void Arm7tdmi::MSR_REG(u32 op) {

}

void Arm7tdmi::MRS(u32 op) {

}