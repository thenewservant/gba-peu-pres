#include "arm/arm7tdmi.h"

#define RN(op) ((op >> 8) & 0x7)
#define RD_LOW(op) (op & 0x7)
#define RD_HIGH(op) ((op >> 8) & 0x7)
#define RB(op) ((op >> 3) & 0x7)

#define MSB_RD(op) (op & BIT(7))
#define MSB_RM(op) (op & BIT(6))

enum HIGH_REG_OPERATION_OPCODE {
	TB_HR_ADD = 0,
	TB_HR_CMP,
	TB_HR_MOV,
	TB_HR_BX
};

//THUMB.05
void Arm7tdmi::TB_HIGH_REG_OPERATION(u16 op) {
	u8 rd = MSB_RD(op) ? (RD_LOW(op) | 0x8) : RD_LOW(op);
	u8 rm = (op >> 3) & (MSB_RM(op) ? 0xF : 0x7);
	u32 rmVal = rRegThumb(rm);

	switch ((op >> 8) & 0x3) {
	case TB_HR_ADD:
		wReg(rd, rRegThumb(rd) + rmVal);
		break;
	case TB_HR_CMP:
	{
		u32 armOp = 0b11100001010100000000000000000000;
		armOp |= (rd << 16) | rm;
		CMP(armOp);
	}
	break;
	case TB_HR_MOV:
		wReg(rd, rmVal);
		break;
	case TB_HR_BX:
		if (rm == 15) {
			rmVal &= ~2; // Auto-align
		}
		cpsr &= ~T;
		cpsr |= (rmVal & 0x1) ? T : 0; // Set T bit to bit 0 of Rm
		wReg(15, (u32)((rmVal & 0xFFFFFFFE))); // Clear the bottom two bits of the adress
		break;
	}
}

//THUMB.06
void Arm7tdmi::TB_LDRPC(u16 op) {
	u32 adress = (rRegThumb(15) & 0xFFFFFFFC) + ((op & 0xFF) * 4);
	wReg(RD_HIGH(op), bus->read32(adress));
}

enum LDR_STR_RELATIVE_OPCODE {
	TB_REL_STR = 0,
	TB_REL_STRB,
	TB_REL_LDR,
	TB_REL_LDRB
};

//THUMB.07
void Arm7tdmi::TB_LDR_STR_RELATIVE(u16 op) {
	u32 rnVal = rRegThumb(RB(op));
	u32 rmVal = rRegThumb((op >> 6) & 0x7);
	u32 adress = rnVal + rmVal;
	u8 data = 0;
	u8 opcode = (op >> 10) & 0x3;
	switch (opcode) {
	case TB_REL_STR:
		bus->write32(adress, rRegThumb(RD_LOW(op)));
		break;
	case TB_REL_STRB:
		data = (u8)rRegThumb(RD_LOW(op));
		bus->write8(adress, data);
		break;
	case TB_REL_LDR:
	{
		u32 armOp = 0b11100111100100000000000000000000;
		armOp |= (RD_LOW(op) << 12) | (RB(op) << 16) | ((op >> 6) & 0x7);
		LDR(armOp);
	}
	break;
	case TB_REL_LDRB:
		wReg(RD_LOW(op), bus->read8(adress));
		break;
	default:
		break;
	}
}

enum LDR_STR_SE_HW_OPCODE {
	TB_SE_STRH = 0,
	TB_SE_LDSB,
	TB_SE_LDRH,
	TB_SE_LDSH
};

//THUMB.08
// LDR/STR with Sign-extension and halfword/byte transfer
void Arm7tdmi::TB_LDR_STR_SE_HW(u16 op) {
	u32 armOp = 0b11100001100000000000000010010000;
	armOp |= (RD_LOW(op) << 12) | (RB(op) << 16) | ((op >> 6) & 0x7);
	switch ((op & 0x0C00) >> 10) {
	case TB_SE_STRH:
		armOp |= 0b10110000;
		STR2(armOp);
		return;
	case TB_SE_LDSB:
		armOp |= 0b100000000000011010000;
		break;
	case TB_SE_LDRH:
		armOp |= 0b100000000000010110000;
		break;
	case TB_SE_LDSH:
		armOp |= 0b100000000000011110000;
		break;
	default:
		break;
	}
	LDR2(armOp);
}

enum LDR_STR_IMMEDIATE_OPCODE {
	TB_STR_IMM = 0,
	TB_LDR_IMM,
	TB_STRB_IMM,
	TB_LDRB_IMM
};
//THUMB.09
//immediate offset
void Arm7tdmi::TB_LDR_STR_IMMEDIATE(u16 op) {
	u32 rnVal = rRegThumb((op >> 3) & 0x7);
	u8 rd = RD_LOW(op);
	u32 adress = 0;
	u8 immed5 = (((op >> 6) & 0x1F));
	switch ((op >> 11) & 0x3) {
	case TB_STR_IMM:
		adress = rnVal + immed5 * 4;
		bus->write32(adress, rRegThumb(rd));
		break;
	case TB_LDR_IMM:
	{
		u32 armOp = 0b11100101100100000000000000000000;
		armOp |= (rd << 12) | (((op >> 3) & 0x7) << 16) | (immed5 << 2);
		LDR(armOp);
	}
	break;
	case TB_STRB_IMM:
		adress = rnVal + immed5;
		bus->write8(adress, (u8)rRegThumb(rd));
		break;
	case TB_LDRB_IMM:
		adress = rnVal + immed5;
		wReg(rd, bus->read8(adress));
		break;
	default:
		break;
	}
}

//THUMB.10
void Arm7tdmi::TB_LDRH_STRH(u16 op) {
	u8 rd = RD_LOW(op);
	u8 rn = RB(op);
	u8 immed5 = (op >> 6) & 0x1F;
	u32 armOp = 0b11100001110000000000000010110000;
	armOp |= (rd << 12) | (rn << 16);
	u8 immed5Bits43 = (immed5 >> 3) & 0x3;
	u8 immed5Bits210 = immed5 & 0x7;
	armOp |= (immed5Bits43 << 8) | (immed5Bits210 << 1);

	if (op & BIT(11)) { //LDRH, else STRH
		armOp |= BIT(20);
		LDR2(armOp);
	}
	else {
		STR2(armOp);
	}
}

//THUMB.11: load / store SP - relative (LDR4/STR3)
void Arm7tdmi::TB_LDRSP_STRSP(u16 op) {
	u32 armOp = 0b11100101100011010000000000000000;
	u8 rd = RD_HIGH(op);
	u8 immed8 = op & 0xFF;
	armOp |= (rd << 12) | (immed8 << 2);
	if (op & BIT(11)) { //LDR
		armOp |= BIT(20);
		LDR(armOp);
	}
	else {
		STR(armOp);
	}
}

//THUMB.12
void Arm7tdmi::TB_GET_REL_ADDR(u16 op) {
	s8 off = (op & 0xFF);
	s32 offset = ((s32)off) * 4;
	u8 rd = RD_HIGH(op);
	if (!(op & BIT(11))) { // ADD (5)
		wReg(rd, (rRegThumb(15) & 0xFFFFFFFC) + offset);

	}
	else { // ADD(6)
		wReg(rd, rRegThumb(13) + (off << 2));

	}
}

//THUMB.13
void Arm7tdmi::TB_ADD_OFFSET_SP(u16 op) {
	u16 offset = (op & 0x7F) << 2;
	if (op & BIT(7)) { // SUB
		wReg(13, rRegThumb(13) - offset);
	}
	else { // ADD
		wReg(13, rRegThumb(13) + offset);
	}
}

//#include <cassert>
//static u32 countSetBits(u16 n) {
//	u32 count = 0;
//	while (n) {
//		count += n & 1;
//		n >>= 1;
//	}
//	return count;
//}

//THUMB.14
void Arm7tdmi::TB_PUSH(u16 op) {
	u32 armOp = 0b11101001001011010000000000000000;
	armOp |= (op & BIT(8)) ? BIT(14) : 0;
	armOp |= op & 0xFF;
	STM(armOp);
}


//THUMB.14
void Arm7tdmi::TB_POP(u16 op) {
	u32 adress = rRegThumb(13);

	for (int i = 0; i < 8; i++) {
		if (op & BIT(i)) {
			wReg(i, bus->read32(adress));
			adress += 4;
		}
	}

	if (op & BIT(8)) {//BIT R
		u32 value = bus->read32(adress);
		wReg(15, (value & 0xFFFFFFFE));
		adress += 4;
	}
	//assert((adress == (rRegThumb(13) + 4 * countSetBits(op&0x1ff))));
	wReg(13, adress);
}

//THUMB.15
void Arm7tdmi::TB_LDMIA_STMIA(u16 op) {
	u8 rn = RN(op);
	u8 regList = op & 0xFF;
	u32 armOp = 0b11101000100000000000000000000000;
	armOp |= (rn << 16) | regList;
	if (op & BIT(11)) {//LDMIA
		armOp |= BIT(20);
		if (!((regList >> rn) & 0x1)) {
			armOp |= BIT(21);
		}
		LDM(armOp);
	}
	else {//STMIA
		armOp |= BIT(21);
		STM(armOp);
	}
}
