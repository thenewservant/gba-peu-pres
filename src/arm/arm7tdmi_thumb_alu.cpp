#include "arm/arm7tdmi.h"

/*
arm7tdmi_thumb_alu.cpp: alu operations, move shifted register,
add/subtract, add/subtract with carry, compare, logical operations...
*/
#define RD(op)			(op & 0x7)
#define RS(op)			((op >> 3) & 0x7)
#define ALU_OPCODE(op)	((op >> 6) & 0xF)

enum THUMB_MOVE_SHIFTED_REGISTER_OPCODES {
	TB_MOVE_LSL = 0x0,
	TB_MOVE_LSR = 0x1,
	TB_MOVE_ASR = 0x2,
};

//THUMB.01
void Arm7tdmi::TB_MOVE_SHIFTED_REG(u16 op) { //MOV(2)
	u32 armOp = 0b11100001101100000000000000000000;
	armOp |= (op & (BIT(11) | BIT(12))) >> 6; // opcode
	armOp |= RD(op) << 12; //RD
	armOp |= RS(op); //RM
	armOp |= ((op >> 6) & 0x1F) << 7; //immed_5
	MOV(armOp);
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
	switch (ALU_OPCODE(op)) {
	case TB_ALU_AND:
	{
		u32 armOp = 0b11100000000100000000000000000000;
		armOp |= (rd << 16) | (rd << 12) | RS(op);
		AND(armOp);
		return;
	}
	case TB_ALU_EOR:
	{
		u32 armOp = 0b11100000001100000000000000000000;
		armOp |= (rd << 16) | (rd << 12) | RS(op);
		EOR(armOp);
		return;
	}
	case TB_ALU_LSL:
	{
		u32 armOp = 0b11100001101100000000000000010000;
		armOp |= (rd << 12) | rd | (RS(op) << 8);
		MOV(armOp);
		return;
	}
	case TB_ALU_LSR:
	{
		u32 armOp = 0b11100001101100000000000000110000;
		armOp |= (rd << 12) | rd | (RS(op) << 8);
		MOV(armOp);
		return;
	}
	case TB_ALU_ASR:
	{
		u32 armOp = 0b11100001101100000000000001010000;
		armOp |= (rd << 12) | rd | (RS(op) << 8);
		MOV(armOp);
		return;
	}
	case TB_ALU_ADC:
	{
		u32 armOp = 0b11100000101100000000000000000000;
		armOp |= (rd << 16) | (rd << 12) | RS(op);
		ADC(armOp);
		return;
	}
	case TB_ALU_SBC:
	{
		u32 armOp = 0b11100000110100000000000000000000;
		armOp |= (rd << 16) | (rd << 12) | RS(op);
		SBC(armOp);
		return;
	}
	case TB_ALU_ROR:
	{
		u32 armOp = 0b11100001101100000000000001110000;
		armOp |= (rd << 12) | rd | (RS(op) << 8);
		MOV(armOp);
		return;
	}
	case TB_ALU_TST:
	{
		u32 armOp = 0b11100001000100000000000000000000;
		armOp |= (rd << 16) | RS(op);
		TST(armOp);
		return;
	}
	case TB_ALU_NEG:
	{
		u32 armOp = 0b11100010011100000000000000000000;
		armOp |= (rd << 12) | (RS(op) << 16);
		RSB(armOp);
		return;
	}
	break;
	case TB_ALU_CMP:
	{
		u32 armOp = 0b11100001010100000000000000000000;
		armOp |= (rd << 16) | RS(op);
		CMP(armOp);
		return;
	}
	case TB_ALU_CMN:
	{
		u32 armOp = 0b11100001011100000000000000000000;
		armOp |= (rd << 16) | RS(op);
		CMN(armOp);
		return;
	}
	break;
	case TB_ALU_ORR:
	{
		u32 armOp = 0b11100001100100000000000000000000;
		armOp |= (rd << 16) | (rd << 12) | RS(op);
		ORR(armOp);
	}
	break;
	case TB_ALU_MUL:
	{
		u32 armOp = 0b11100000000100000000000010010000;
		armOp |= (rd << 16) | (rd << 8) | RS(op);
		MUL(armOp);
		break;
	}
	case TB_ALU_BIC:
	{
		u32 armOp = 0b11100001110100000000000000000000;
		armOp |= (rd << 16) | (rd << 12) | RS(op);
		BIC(armOp);
	}
	break;
	case TB_ALU_MVN:
	{
		u32 armOp = 0b11100001111100000000000000000000;
		armOp |= (rd << 12) | RS(op);
		MVN(armOp);
	}
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
	u8 immed8 = op & 0xFF;
	switch ((op >> 11) & 0x3) {
	case TB_IMM_MOV:
	{
		u32 armOp = 0b11100011101100000000000000000000;
		armOp |= (rd << 12) | immed8;
		MOV(armOp);
	}
	return;
	case TB_IMM_CMP:
	{
		u32 armOp = 0b11100011010100000000000000000000;
		armOp |= (rd << 16) | immed8;
		CMP(armOp);
	}
	return;
	case TB_IMM_ADD:
	{
		u32 armOp = 0b11100010100100000000000000000000;
		armOp |= (rd << 16) | (rd << 12) | immed8;
		ADD(armOp);
	}
	return;
	case TB_IMM_SUB:
	{
		u32 armOp = 0b11100010010100000000000000000000;
		armOp |= (rd << 16) | (rd << 12) | immed8;
		SUB(armOp);
	}
	return;
	default:
		break;
	}
}

//THUMB.2
void Arm7tdmi::TB_ADD_SUBSTRACT(u16 op) {
	u32 armOp = 0b11100000000100000000000000000000;
	armOp |= (op & BIT(9)) ? BIT(22) : BIT(23); //SUB or ADD
	armOp |= (op & BIT(10)) ? BIT(25) : 0; // immed_3 or register
	armOp |= ((op >> 3) & 0x7) << 16; //RN
	armOp |= (op & 0x7) << 12; //RD
	armOp |= ((op >> 6) & 0x7) ; //RM or immed_3
	if (op & BIT(9)) {
		SUB(armOp);
	}
	else {
		ADD(armOp);
	}
}
