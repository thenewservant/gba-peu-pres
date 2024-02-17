#include "arm7tdmi.h"

/*
arm7tdmi_thumb_alu.cpp: alu operations, move shifted register,
add/subtract, add/subtract with carry, compare, logical operations...
*/
#define RD(op)			(op & 0x7)
#define RS(op)			((op >> 3) & 0x7)
#define MOVE_OPCODE(op) ((op >>11) & 0x3)
#define ALU_OPCODE(op)	((op >> 6) & 0xF)
//should be used only after assignments are done
#define CHECKCPSR_NZ cpsr = (cpsr & ~N) | ((result & BIT(31)) ? N : 0); \
					 cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0)

#define ASSIGN_TO_REG_NZ wReg(rd, result); \
					 CHECKCPSR_NZ

enum TB_MOVE_SHIFTED_REGISTER_OPCODES {
	TB_MOVE_LSL = 0x0,
	TB_MOVE_LSR = 0x1,
	TB_MOVE_ASR = 0x2,
};

//THUMB.01
void Arm7tdmi::TB_MOVE_SHIFTED_REG(u16 op) { //MOV(2)
	u32 rmVal = rReg(RS(op));
	u8 rd = RD(op);
	u32 immed5 = (op >> 6) & 0x1F;
	u32 result = 0;

	switch ((op >> 11) & 0x3) {
	case TB_MOVE_LSL:
		if (immed5) {
			result = rmVal << immed5;
			cpsr = (cpsr & ~C) | ((rmVal & BIT(32 - immed5)) ? C : 0);
		}
		else {
			result = rmVal;
		}
		break;
	case TB_MOVE_LSR:
		if (immed5) {
			result = rmVal >> immed5;
			cpsr = (cpsr & ~C) | ((rmVal & BIT(immed5 - 1)) ? C : 0);
		}
		else {
			result = 0;
			cpsr = (cpsr & ~C) | ((rmVal & BIT(31)) ? C : 0);
		}
		break;
	case TB_MOVE_ASR:
		if (immed5) {
			result = rmVal >> immed5;
			cpsr = (cpsr & ~C) | ((rmVal & BIT(immed5 - 1)) ? C : 0);
		}
		else {
			bool bit31 = (rmVal & BIT(31)) > 0;
			cpsr = (cpsr & ~C) | ((bit31) ? C : 0);
			result = bit31 ? 0xFFFFFFFF : 0;
		}
		break;
	default:
		break;
	}
	cpsr &= ~N & ~Z;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= (result & BIT(31)) ? N : 0;
	wReg(rd, result);
}

enum THUMB_ALU_OPCODE {
	TB_AND = 0x0,
	TB_EOR = 0x1,
	TB_LSL = 0x2,
	TB_LSR = 0x3,
	TB_ASR = 0x4,
	TB_ADC = 0x5,
	TB_SBC = 0x6,
	TB_ROR = 0x7,
	TB_TST = 0x8,
	TB_NEG = 0x9,
	TB_CMP = 0xA,
	TB_CMN = 0xB,
	TB_ORR = 0xC,
	TB_MUL = 0xD,
	TB_BIC = 0xE,
	TB_MVN = 0xF
};

void Arm7tdmi::TB_ALU_OP(u16 op) {
	u8 rd = RD(op);
	u32 rdVal = rReg(rd);
	u32 rsVal = rReg(RS(op));
	u32 result = 0;

	switch (ALU_OPCODE(op)) {
	case TB_AND:
		result = rdVal & rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_EOR:
		result = rdVal ^ rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_LSL:
		result = rdVal ^ rsVal;
		//ASSIGN_TO_REG_NZC;
		break;
	case TB_LSR:
		break;
	case TB_ASR:
		break;
	case TB_ADC:
		break;
	case TB_SBC:
		break;
	case TB_ROR:
		break;
	case TB_TST:
		result = rdVal & rsVal;
		CHECKCPSR_NZ;
		break;
	case TB_NEG:
		result = rdVal & rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_CMP:
		break;
	case TB_CMN:
		result = rdVal + rsVal;
		CHECKCPSR_NZ;
		break;
	case TB_ORR:
		result = rdVal | rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_MUL:
		result = rdVal * rsVal;
		ASSIGN_TO_REG_NZ;
		cpsr = (cpsr & ~C);
		break;
	case TB_BIC:
		result = rdVal & ~rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_MVN:
		result = ~rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	default:
		break;
	}
}