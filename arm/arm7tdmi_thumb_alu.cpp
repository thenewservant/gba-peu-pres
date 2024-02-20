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

enum THUMB_MOVE_SHIFTED_REGISTER_OPCODES {
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
	TB_ALU_AND = 0x0,
	TB_ALU_EOR = 0x1,
	TB_ALU_LSL = 0x2,
	TB_ALU_LSR = 0x3,
	TB_ALU_ASR = 0x4,
	TB_ALU_ADC = 0x5,
	TB_ALU_SBC = 0x6,
	TB_ALU_ROR = 0x7,
	TB_ALU_TST = 0x8,
	TB_ALU_NEG = 0x9,
	TB_ALU_CMP = 0xA,
	TB_ALU_CMN = 0xB,
	TB_ALU_ORR = 0xC,
	TB_ALU_MUL = 0xD,
	TB_ALU_BIC = 0xE,
	TB_ALU_MVN = 0xF
};

void Arm7tdmi::TB_ALU_OP(u16 op) {
	u8 rd = RD(op);
	u32 rdVal = rReg(rd);
	u32 rsVal = rReg(RS(op));
	u32 result = 0;

	switch (ALU_OPCODE(op)) {
	case TB_ALU_AND:
		result = rdVal & rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_ALU_EOR:
		result = rdVal ^ rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_ALU_LSL:
		result = rdVal ^ rsVal;
		ASSIGN_TO_REG_NZ;
		//ASSIGN_TO_REG_NZC;
		break;
	case TB_ALU_LSR:
		
	case TB_ALU_ASR:

	case TB_ALU_ADC:
		
	case TB_ALU_SBC:
		
	case TB_ALU_ROR:
		printf("unimplemented\n");
		exit(56);
	case TB_ALU_TST:
		result = rdVal & rsVal;
		CHECKCPSR_NZ;
		break;
	case TB_ALU_NEG:
		result = rdVal & rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_ALU_CMP:
		result = rdVal - rsVal;
		CHECKCPSR_NZ;
	case TB_ALU_CMN:
		result = rdVal + rsVal;
		CHECKCPSR_NZ;
		break;
	case TB_ALU_ORR:
		result = rdVal | rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_ALU_MUL:
		result = rdVal * rsVal;
		ASSIGN_TO_REG_NZ;
		cpsr = (cpsr & ~C);
		break;
	case TB_ALU_BIC:
		result = rdVal & ~rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_ALU_MVN:
		result = ~rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	default:
		break;
	}
}

enum THUMB_IMMEDIATE_OPCODE {
	TB_IMM_MOV = 0,
	TB_IMM_CMP = 1,
	TB_IMM_ADD = 2,
	TB_IMM_SUB = 3
};

void Arm7tdmi::TB_IMMEDIATE_OPERATION(u16 op) {
	u8 rd = (op >> 8) & 0x7;
	u8 immed8 = op & 0xFF;
	u32 result = 0;
	switch ((op >> 11) & 0x3) {
		case TB_IMM_MOV:
			result = immed8;
			CHECKCPSR_NZ;
			break;
		case TB_IMM_CMP:
			result = rReg(rd) - immed8;
			CHECKCPSR_NZ;
			//MISSING CV
			result = rReg(rd); // real result will not be set to rd
			break;
		case TB_IMM_ADD:
			result = rReg(rd) + immed8;
			CHECKCPSR_NZ;
			//MISSING CV
			break;
		case TB_IMM_SUB:
			result = rReg(rd) - immed8;
			CHECKCPSR_NZ;
			//MISSING CV
			break;
		default:
			break;
	}
	wReg(rd, result);
}

void Arm7tdmi::TB_ADD_SUBSTRACT(u16 op) {
	u8 rd = RD(op);
	u32 rsVal = rReg(RS(op));
	u32 stuffToAdd = (op & BIT(10)) ? (op >> 6) & 0x7 : rReg((op >> 6) & 0x7);
	u32 result = 0;

	if (op & BIT(9)) { //subtract
		result = rsVal - stuffToAdd;
	}
	else { // add
		result = rsVal + stuffToAdd;
	}
	wReg(rd, result);
	cpsr &= ~N & ~Z & ~C & ~V;
	cpsr |= (result == 0) ? Z : 0;
	cpsr |= (result & BIT(31)) ? N : 0;
	//C flag computing
	cpsr |= (op & BIT(9)) ? (rsVal >= stuffToAdd) ? C : 0 : (rsVal + stuffToAdd) < rsVal ? C : 0;
	//V flag computing
	cpsr |= (op & BIT(9)) ? ((rsVal & BIT(31)) != (stuffToAdd & BIT(31)) && (rsVal & BIT(31)) != (result & BIT(31))) ? V : 0 : ((rsVal & BIT(31)) == (stuffToAdd & BIT(31)) && (rsVal & BIT(31)) != (result & BIT(31))) ? V : 0;
}