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
	u32 rmVal = rRegThumb(RS(op));
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
			result = ((s32)rmVal) >> immed5;
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
	u8 rd =  RD(op);
	u32 rdVal = rRegThumb(rd);
	u32 rsVal = rRegThumb(RS(op));
	u32 result = 0;

	switch (ALU_OPCODE(op)) {
	case TB_ALU_AND:
		result = rdVal & rsVal;
		wReg(rd, result);
		cpsr = (cpsr & ~N) | ((result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
		break;
	case TB_ALU_EOR:
		result = rdVal ^ rsVal;
		ASSIGN_TO_REG_NZ;
		break;
	case TB_ALU_LSL:
	{
		result = rdVal;
		u8 rsVal8 = (u8)rsVal;
		if (rsVal8 > 0 && rsVal < 32) {
			cpsr = (cpsr & ~C) | ((rdVal & BIT(32 - rsVal8)) ? C : 0);
			result = rdVal << rsVal8;
		}
		else if (rsVal8 == 32) {
			cpsr = (cpsr & ~C) | ((rdVal & BIT(0)) ? C : 0);
			result = 0;
		}
		else if (rsVal8 > 32) {
			cpsr = (cpsr & ~C);
			result = 0;
		}
		wReg(rd, result);
		cpsr = (cpsr & ~N) | ((result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
	}
	break;
	case TB_ALU_LSR:
	{
		result = rdVal;
		u8 rsVal8 = (u8)rsVal;
		if (rsVal8 > 0 && rsVal < 32) {
			cpsr = (cpsr & ~C) | (rdVal & BIT(rsVal8 - 1) ? C : 0);
			result = rdVal >> rsVal8;
		}
		else if (rsVal8 == 32) {
			cpsr = (cpsr & ~C) | ((rdVal & BIT(31)) ? C : 0);
			result = 0;
		}
		else if (rsVal8 > 32) {
			cpsr = (cpsr & ~C);
			result = 0;
		}
		wReg(rd, result);
		cpsr = (cpsr & ~N) | ((result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
	}
	break;
	case TB_ALU_ASR:
	{
		result = rdVal;
		u8 rsVal8 = (u8)rsVal;
		if (rsVal8 > 0 && rsVal < 32) {
			cpsr = (cpsr & ~C) | ((rdVal & BIT(rsVal8 - 1)) ? C : 0);
			result = (s32)rdVal >> rsVal8;
		}
		else if (rsVal8 >= 32) {
			cpsr = (cpsr & ~C) | ((rdVal & BIT(31)) ? C : 0);
			if (rdVal & BIT(31)) {
				result = 0xFFFFFFFF;
			}
			else {
				result = 0;
			}
		}

		wReg(rd, result);
		cpsr = (cpsr & ~N) | ((result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
	}
	break;
	case TB_ALU_ADC:
	{
		u64 result = (u64)rdVal + rsVal + (u64)((cpsr & C) > 0);
		cpsr = (cpsr & ~N) | (((u32)result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | (((u32)result == 0) ? Z : 0);
		cpsr = (cpsr & ~C) | ((result > 0xFFFFFFFF) ? C : 0); // unsafe
		cpsr = (cpsr & ~V) | (((((rdVal ^ result) & (rsVal ^ result)) >> 31) & 1) ? V : 0);
		wReg(rd, result);
	}
	break;
	case TB_ALU_SBC:
	{
		u64 result = (u64)rdVal + ~rsVal + (u64)((cpsr & C) > 0);
		cpsr = (cpsr & ~N) | (((u32)result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | (((u32)result == 0) ? Z : 0);
		cpsr = (cpsr & ~C) | ((result > 0xFFFFFFFF) ? C : 0); // unsafe
		cpsr = (cpsr & ~V) | (((((rdVal ^ result) & (~rsVal ^ result)) >> 31) & 1) ? V : 0);
		wReg(rd, result);
	}
	break;
	case TB_ALU_ROR:
	{
		result = rdVal;
		u8 rsVal8 = (u8)rsVal;
		if (rsVal8 == 0) {
		}
		else if ((rsVal8 & 0x1F) == 0) {
			cpsr = (cpsr & ~C) | ((rdVal & BIT(31)) ? C : 0);
		}
		else {
			cpsr = (cpsr & ~C) | ((rdVal & BIT((rsVal8 & 0x1F) - 1)) ? C : 0);
			result = (rdVal >> (rsVal8 & 0x1F)) | (rdVal << (32 - (rsVal8 & 0x1F)));
		}

		wReg(rd, result);
		cpsr = (cpsr & ~N) | ((result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
	}
	break;
	case TB_ALU_TST:
		result = rdVal & rsVal;
		CHECKCPSR_NZ;
		break;
	case TB_ALU_NEG:
	{
		rdVal = 0;
		u64 result = (u64)rdVal - rsVal;
		cpsr = (cpsr & ~N) | (((u32)result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | (((u32)result == 0) ? 0 : 0);
		cpsr = (cpsr & ~C) | ((result > 0xFFFFFFFF) ? C : 0); // unsafe
		cpsr = (cpsr & ~V) | (((((rdVal ^ result) & (rdVal ^ rsVal)) >> 31) & 1) ? V : 0);
		wReg(rd, result);
	}
	break;
	case TB_ALU_CMP:
		result = rdVal - rsVal;
		cpsr = (cpsr & ~N) | ((result & (1 << (31))) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result > 0) ? 0 : Z);
		cpsr = (cpsr & ~C) | ((rdVal >= rsVal) ? C : 0); // unsafe
		cpsr = (cpsr & ~V) | (((((rdVal ^ result) & (rsVal ^ rdVal)) >> 31) & 1) ? V : 0);
		break;
	case TB_ALU_CMN:
	{
		u64 result = (u64)rdVal + rsVal;
		cpsr = (cpsr & ~N) | (((u32)result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | (((u32)result == 0) ? Z : 0);
		cpsr = (cpsr & ~C) | ((result > 0xFFFFFFFF) ? C : 0); // unsafe
		cpsr = (cpsr & ~V) | (((((rdVal ^ result) & (rsVal ^ result)) >> 31) & 1) ? V : 0);
	}
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
		wReg(rd, result);
		cpsr = (cpsr & ~N) | ((result & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
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

void Arm7tdmi::TB_IMMEDIATE_OPERATION(u16 op) { //Will be refactored soon. 
	u8 rd = (op >> 8) & 0x7;
	u32 rdVal = rRegThumb(rd);
	u8 immed8 = op & 0xFF;
	u32 result = 0;
	switch ((op >> 11) & 0x3) {
	case TB_IMM_MOV:
		result = immed8;
		cpsr = (cpsr & ~N) | ((result & (1 << (31))) ? N : 0); cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
		wReg(rd, result);
		break;
	case TB_IMM_CMP:
	{
		u32 rsVal = immed8;
		result = rdVal - rsVal;
		cpsr = (cpsr & ~N) | ((result & (1 << (31))) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result > 0) ? 0 : Z);
		cpsr = (cpsr & ~C) | ((rdVal >= rsVal) ? C : 0); // unsafe
		cpsr = (cpsr & ~V) | (((((rdVal ^ result) & (rsVal ^ result)) >> 31) & 1) ? V : 0);
	}
	return;
	case TB_IMM_ADD:
		result = rdVal + immed8;
		{
			u64 result2 = (u64)rdVal + immed8;
			cpsr = (cpsr & ~N) | ((result & (1 << (31))) ? N : 0); cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
			cpsr = (cpsr & ~C) | ((result2 > 0xFFFFFFFF) ? C : 0); // unsafe
			cpsr = (cpsr & ~V) | (((immed8 ^ result) & (rdVal ^ result) & BIT(31)) ? V : 0);
		}

		break;
	case TB_IMM_SUB:
	{
		u32 imm8_32 = ~(u32)immed8 + 1;
		result = rdVal + imm8_32;
		cpsr = (cpsr & ~N) | ((result & (1 << (31))) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0);
		cpsr = (cpsr & ~C) | ((rdVal >= immed8) ? C : 0);
		cpsr = (cpsr & ~V) | (((imm8_32 ^ result) & (rdVal ^ result) & BIT(31)) ? V : 0);
	}

	break;
	default:
		break;
	}
	wReg(rd, result);
}

void Arm7tdmi::TB_ADD_SUBSTRACT(u16 op) {
	u8 rd = RD(op);
	u64 rsVal = rRegThumb(RS(op));
	u32 operand = (op & BIT(10)) ? (op >> 6) & 0x7 : rRegThumb((op >> 6) & 0x7);
	u64 result = 0;
	cpsr &= ~N & ~Z & ~C & ~V;
	if (op & BIT(9)) { //subtract
		result = rsVal - operand;
		cpsr |= ((((rsVal ^ operand) & (rsVal ^ result)) >> 31) & 1) ? V : 0;
		cpsr |= (result > 0xFFFFFFFF) ? 0 : C;
	}
	else { // add
		result = rsVal + operand;
		cpsr |= (((~(rsVal ^ operand) & (rsVal ^ result)) >> 31) & 1) ? V : 0;
		cpsr |= (result > 0xFFFFFFFF) ? C : 0;
	}
	wReg(rd, result);
	
	cpsr |= ((u32)result & BIT(31)) ? N : 0;
	cpsr |= ((u32)result == 0) ? Z : 0;
	
	
}