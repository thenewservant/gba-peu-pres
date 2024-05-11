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

#define CPSR_UPDATE if (BIT_S(op) && (RD(op) == 15)) {\
						if (CURRENT_MODE_HAS_SPSR) {\
							cpsr = getSPSRValue();\
						}\
					}\
					else if (BIT_S(op)) {\
						cpsr &= ~N & ~Z & ~C & ~V;\
						cpsr |= ((u32)result & BIT(31)) ? N : 0;\
						cpsr |= ((u32)result == 0) ? Z : 0;\
						cpsr |= (result > 0xFFFFFFFF) ? C : 0;\
						cpsr |= ((((rnVal ^ result) & (operand2Val ^ result)) >> 31) & 1) ? V : 0;\
					}

enum OPERAND2_SHIFT_TYPE {
	LSL_IMM = 0,
	LSL_REG,
	LSR_IMM,
	LSR_REG,
	ASR_IMM,
	ASR_REG,
	ROR_IMM,
	ROR_REG
};

u32 operand2(Arm7tdmi* cpu, u32 op, u32 cpsr, u8* carryOut) {
	if (BIT_I(op)) {
		u8 rotateImm = (op >> 8) & 0xF;
		u8 imm = op & 0xFF;

		u32 result = (imm >> (2 * rotateImm)) | (imm << (32 - 2 * rotateImm));
		if (rotateImm) {
			*carryOut = (result & BIT(31)) ? 1 : 0;
		}
		else {
			*carryOut = (cpsr & C) ? 1 : 0;
		}
		return result;
	}
	else { // Register as 2nd operand

		u8 shiftType = (op >> 5) & 0x3;
		u8 shiftAmount = (op >> 7) & 0x1F;
		u8 rm = op & 0xF;
		u8 strategy = ((op >> 4) & 0x7);
		
		u32 rmVal ;

		if ((strategy & 1) && (rm == 15)) {
			rmVal = cpu->rReg(rm) + 4;
		}
		else {
			rmVal = cpu->rReg(rm);
		}

		u32 cpsrVal = cpsr;
		if (((op >> 4) & 0xFF) == 0) { //Data-processing operands - Register
			*carryOut = (cpsrVal & C) ? 1 : 0;
			return rmVal;
		}
		else if (((op >> 4) & 0x7) == LSL_IMM) { // Logical shift left by immediate
			if (shiftAmount) {
				*carryOut = (rmVal & BIT(32 - shiftAmount))?1:0;
				return rmVal << shiftAmount;
			}
			else {
				*carryOut = (cpsrVal & C) ? 1 : 0;
				return rmVal;
			}
		}
		else if (((op >> 4) & 0x7) == LSL_REG){
			u32 rsVal = cpu->rReg((op >> 8) & 0xF);
			u8 rsVal8 = (u8)rsVal;
			if (!rsVal8) {
				*carryOut = (cpsrVal & C) ? 1 : 0;
				return rmVal;
			}
			else if (rsVal8 < 32) {
				*carryOut = (rmVal & BIT(32 - rsVal8)) ? 1 : 0;
				return rmVal<< rsVal8;
			}
			else if (rsVal8 == 32) {
				*carryOut = (rmVal & BIT(0)) ? 1 : 0;
				return 0;
			}
			else { // rsVal8 > 32
				*carryOut = 0;
				return 0;
			}
		}
		else if (((op >> 4) & 0x7) == LSR_IMM) {
			if (shiftAmount) {
				*carryOut = (rmVal & BIT(shiftAmount - 1)) ? 1 : 0;
				return rmVal >> shiftAmount;
			}
			else {
				*carryOut = ( (rmVal & BIT(31)) >0 )? 1 : 0;
				return 0;
			}
		}
		else if (((op >> 4) & 0x7) == LSR_REG) {
			u32 rsVal = cpu->rReg((op >> 8) & 0xF);
			u8 rsVal8 = (u8)rsVal;
			if (!rsVal8) {
				*carryOut = (cpsrVal & C) ? 1 : 0;
				return rmVal;
			}
			else if (rsVal8 < 32) {
				*carryOut = (rmVal & BIT(rsVal8 - 1)) ? 1 : 0;
				return rmVal >> rsVal8;
			}
			else if (rsVal8 == 32) {
				*carryOut = (rmVal & BIT(31)) ? 1 : 0;
				return 0;
			}
			else { // rsVal8 >= 32
				*carryOut = 0;
				return 0;
			}
		}
		else if (((op >> 4) & 0x7) == ASR_IMM) {
			if (shiftAmount) {
				*carryOut = (rmVal & BIT(shiftAmount - 1)) ? 1 : 0;
				return ((s32)rmVal) >> shiftAmount;
			}
			else {
				*carryOut = (rmVal & BIT(31)) ? 1 : 0;
				if (rmVal & BIT(31)) {
					return 0xFFFFFFFF;
				}
				return 0;
			}
		}
		else if (((op >> 4) & 0x7) == ASR_REG) {
			u32 rsVal = cpu->rReg((op >> 8) & 0xF);
			u8 rsVal8 = (u8)rsVal;
			if (!rsVal8) {
				*carryOut = (cpsrVal & C) ? 1 : 0;
				return rmVal;
			}
			else if (rsVal8 < 32) {
				*carryOut = (rmVal & BIT(rsVal8 - 1) ) ? 1 : 0;
				return rmVal >> rsVal8;
			}
			else {// rsVal8 >= 32
				*carryOut = (rmVal & BIT(31)) ? 1 : 0;
				if (rmVal & BIT(31)) {
					return 0xFFFFFFFF;
				}
				return 0;
			}
		}
		else if (((op >> 4) & 0xFF) == ROR_IMM) {
			*carryOut = rmVal & BIT(0);
			return ((cpsrVal & C) ? 0x80000000 : 0) | (rmVal >> 1);
		}
		else if (((op >> 4) & 0x7) == 6) {
			if (shiftAmount) {
				*carryOut =(( rmVal & BIT(shiftAmount - 1) ) > 0 )? 1 : 0;
				u32 rotatedValue = rmVal >> shiftAmount;
				return (rotatedValue | (rmVal << (32 - shiftAmount)));
			}
		}
		else if (((op >> 4) & 0x7) == ROR_REG) {
			u32 rsVal = cpu->rReg((op >> 8) & 0xF);
			u8 rsVal8 = (u8)rsVal;
			u8 rsVal4 = rsVal8 & 0xF;
			if (!rsVal8) {
				*carryOut = (cpsrVal & C) ? 1 : 0;
				return rmVal;
			}
			else if (!(rsVal & 0xF)) {
				*carryOut = (rmVal & BIT(31)) ? 1 :0;
				return rmVal;
			}
			else { //rotate right by rsVal4
				*carryOut = (rmVal & BIT(rsVal4 - 1)) ? 1 : 0;
				u32 rotatedValue = rmVal >> rsVal4;
				return (rotatedValue | (rmVal << (32 - rsVal4)));

			}
		}
	}
}

void Arm7tdmi::checkCPSR_DP(u32& op, const u8& carryOut) {
	u32 rdVal = rReg(RD(op));
	if ((op & BIT_S(op)) && (RD(op) == 15)) {
		if (CURRENT_MODE_HAS_SPSR) {
			cpsr = getSPSRValue();
		}
	}
	else if ((op & BIT_S(op))) {
		cpsr &= ~N & ~Z & ~C;
		cpsr |= (rdVal & BIT(31)) ? N : 0;
		cpsr |= (rdVal == 0) ? Z : 0;
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


void Arm7tdmi::SUB(u32 op) {
	u8 carryOut;
	u32 rnVal = rReg(RN(op));
	u32 shifterOperand = operand2(this, op, cpsr, &carryOut);
	u32 result = rnVal - shifterOperand;
	wReg(RD(op), result);
	if (BIT_S(op) && (RD(op) == 15)) {
			if (CURRENT_MODE_HAS_SPSR) {
					cpsr = getSPSRValue(); 
			}
	}
	else if (BIT_S(op)) {
		cpsr &= ~N & ~Z & ~C & ~V;
		cpsr |= (result & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
		cpsr |= (rnVal >= shifterOperand) ? C : 0;
		cpsr |= ((rnVal ^ shifterOperand) & (rnVal ^ result) & BIT(31)) ? V : 0;
	}
}

void Arm7tdmi::RSB(u32 op) {
	u8 carryOut;
	u32 shifterOperand = rReg(RN(op));
	u32 rnVal = operand2(this, op, cpsr, &carryOut);
	u32 result = rnVal - shifterOperand;
	wReg(RD(op), result);
	if (BIT_S(op) && (RD(op) == 15)) {
		if (CURRENT_MODE_HAS_SPSR) {
			cpsr = getSPSRValue();
		}
	}
	else if (BIT_S(op)) {
		cpsr &= ~N & ~Z & ~C & ~V;
		cpsr |= (result & BIT(31)) ? N : 0;
		cpsr |= (result == 0) ? Z : 0;
		cpsr |= (rnVal >= shifterOperand) ? C : 0;
		cpsr |= ((rnVal ^ shifterOperand) & (rnVal ^ result) & BIT(31)) ? V : 0;
	}
}

void Arm7tdmi::ADD(u32 op) {
	u8 carryOut;
	u32 rnVal = rReg(RN(op));
	u32 operand2Val = operand2(this, op, cpsr, &carryOut);
	u64 result = (u64)rReg(RN(op)) + operand2Val;
	wReg(RD(op), (u32)result);
	CPSR_UPDATE;
}

void Arm7tdmi::ADC(u32 op) {
	u8 carryOut;
	u32 rnVal = rReg(RN(op));
	u32 operand2Val = operand2(this, op, cpsr, &carryOut);
	u64 result = (u64)rnVal + operand2Val + (u64)((cpsr & C) >0);
	wReg(RD(op), (u32)result);
	CPSR_UPDATE;
}

void Arm7tdmi::SBC(u32 op) {
	u8 carryOut;
	u32 rnVal = rReg(RN(op));
	u32 operand2Val = ~operand2(this, op, cpsr, &carryOut);
	u64 result = (u64)rnVal + operand2Val + (u64)((cpsr & C) > 0) ;
	wReg(RD(op), (u32)result);
	CPSR_UPDATE;
}

void Arm7tdmi::RSC(u32 op) {
	u8 carryOut;
	u32 rnVal = rReg(RN(op));
	u32 operand2Val = operand2(this, op, cpsr, &carryOut);
	u64 result = (u64)operand2Val + ~rnVal + (u64)((cpsr & C) > 0);
	wReg(RD(op), (u32)result);
	CPSR_UPDATE;
}

void Arm7tdmi::CMP(u32 op) {
	u8 carryOut;
	u32 rnVal = rReg(RN(op));
	u32 operand2Val = operand2(this, op, cpsr, &carryOut);
	u64 result = (u64)rnVal + ~operand2Val + 1;
	CPSR_UPDATE;
}

void Arm7tdmi::CMN(u32 op) {
	u8 carryOut;
	u32 operand2Val = operand2(this, op, cpsr, &carryOut);
	u32 rnVal = rReg(RN(op));
	u64 result = (u64)rnVal + operand2Val;
	cpsr &= ~N & ~Z & ~C;
	cpsr |= ((u32)result & BIT(31)) ? N : 0;
	cpsr |= ((u32)result == 0) ? Z : 0;
	cpsr |= (result > 0xFFFFFFFF) ? C : 0;
	cpsr |= ((((rnVal ^ result) & (operand2Val ^ result)) >> 31) & 1) ? V : 0;
}


void Arm7tdmi::TST(u32 op) {
	u8 carryOut;
	u32 result = rReg(RN(op)) & operand2(this, op, cpsr, &carryOut);
	cpsr &= ~N & ~Z & ~C;
	cpsr |= (result & BIT(31)) ? N : 0;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |=  carryOut ? C : 0;
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
	u32 result = operand2(this, op, cpsr, &carryOut);
	wReg(RD(op), result);
	
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

#define MSR_FIELD_MASK(op) ((op>>16) & 0xF)
#define MSR_IMM_ROTATE_IMM(op) ((op >> 8) & 0xF)

#define PSR_MASK_UNALLOC 0x0FFFFF00
#define PSR_USER_MASK 0xF0000000
#define PSR_PRIV_MASK 0x0000000F
#define PSR_STATE_MASK 0x00000020
#define BIT_R(op) (op & BIT(22))

void Arm7tdmi::MSR(u32 operand, u32 op) {
	u32 mask = MSR_FIELD_MASK(op);
	u32 byteMask = (mask & 1 ? 0xFF : 0) |
		(mask & 2 ? 0xFF00 : 0) |
		(mask & 4 ? 0xFF0000 : 0) |
		(mask & 8 ? 0xFF000000 : 0);
	u32 finalMask = 0;
	if (!BIT_R(op)) {
		if (CURRENT_MODE != ARM7TDMI_MODE_USER) {//in a privileged mode
			finalMask = byteMask & (PSR_USER_MASK | PSR_PRIV_MASK);
		}
		else {
			finalMask = byteMask & PSR_USER_MASK;
		}
		cpsr = (cpsr & ~finalMask) | (operand & finalMask);
	}
	else {
		if (currentModeHasSPSR() || true) { // else: unpredictable
			finalMask = byteMask & (PSR_USER_MASK | PSR_PRIV_MASK | PSR_STATE_MASK);
			setSPSRValue(getSPSRValue() & ~finalMask | (finalMask & operand));
		}
	}
}

void Arm7tdmi::MSR_IMM(u32 op) {
	u32 operand = ((op & 0xFF) >> (2 * MSR_IMM_ROTATE_IMM(op))) | ((op & 0xFF) << (32 - 2 * MSR_IMM_ROTATE_IMM(op)));//rotated by 2*rotate_im
	MSR(operand, op);
}

void Arm7tdmi::MSR_REG(u32 op) {
	u32 operand = rReg(op & 0xF);
	MSR(operand, op);
}
#define BIT_R(op) (op & BIT(22))

void Arm7tdmi::MRS(u32 op) {
	if (BIT_R(op)) {
		wReg(RD(op), getSPSRValue());
	}
	else {
		wReg(RD(op), cpsr);
	}
}