#include "arm7tdmi.h"

#define RN(op) ((op >> 8) & 0x7)
#define RD_LOW(op) (op & 0x7)
#define RD_HIGH(op) ((op >> 8) & 0x7)
#define RB(op) ((op >> 3) & 0x7)
#define NN(op) ((op >> 6) & 0x1F) // for thumb 10 only
#define RO(op) NN(op)

static u8 countSetBits(u32 n) {
	n = n & 0xFFFF; //16 least significant bits
	u8 count = 0;
	while (n) {
		n &= (n - 1);
		count++;
	}
	return count;
}
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
	u8 rm = (op>>3) & (MSB_RM(op) ? 0xF : 0x7);
	u32 rmVal = rReg(rm);
	if (rd == 15) {
		rmVal += 2;
	}
	switch ((op >> 8) & 0x3) {
	case TB_HR_ADD:
		wReg(rd, rReg(rd) + rmVal);
		break;
	case TB_HR_CMP:
	{
		u32 rdVal = rReg(rd);
		u32 result = rdVal - rmVal;
		cpsr = (cpsr & ~N) | ((result & (1 << (31))) ? N : 0);
		cpsr = (cpsr & ~Z) | ((result > 0) ? 0 : Z);
		cpsr = (cpsr & ~C) | ((rdVal >= rmVal) ? C : 0); // unsafe
		cpsr = (cpsr & ~V) | (((((rdVal ^ result) & (rmVal ^ result)) >> 31) & 1) ? V : 0);
	}

		break;
	case TB_HR_MOV:
		wReg(rd, rmVal);
		break;
	case TB_HR_BX:
		cpsr &= ~T;
		cpsr |= (rmVal & 0x1) ? T : 0; // Set T bit to bit 0 of Rm
		r[15] = (u32)((rmVal & 0xFFFFFFFE)) + ((cpsr & T)?2:6); // Clear the bottom two bits of the address
		break;
	}
}

//THUMB.06
void Arm7tdmi::TB_LDRPC(u16 op) {
	u32 address = (rReg(15) & 0xFFFFFFFC) + ((op & 0xFF) * 4);
	wReg(RD_HIGH(op), bus->read32(address));
}

enum LDR_STR_RELATIVE_OPCODE {
	TB_REL_STR = 0,
	TB_REL_STRB,
	TB_REL_LDR,
	TB_REL_LDRB
};
//THUMB.07
void Arm7tdmi::TB_LDR_STR_RELATIVE(u16 op) {
	u32 rnVal = rReg(RB(op));
	u32 rmVal = rReg((op>>6)&0x7);
	u32 address = rnVal + rmVal;
	printf("address: %08x\n", address);
	u8 data = 0;
	u8 opcode = (op >> 10) & 0x3;
	switch (opcode) {
	case TB_REL_STR:
		bus->write32(address, rReg(RD_LOW(op)));
		break;
	case TB_REL_STRB:
		data = (u8)rReg(RD_LOW(op));
		bus->write8(address, data);
		printf("written: %02x, back read: %08x\n",data, bus->read32(address));
		break;
	case TB_REL_LDR:
		printf("reading from: %08x\n", address);
		printf("read: %08x\n", bus->read32(address));
		wReg(RD_LOW(op), bus->read32(address));
		printf("reg: %08x\n", rReg(RD_LOW(op)));
		break;
	case TB_REL_LDRB:
		wReg(RD_LOW(op), bus->read8(address));
		break;
	default:
		break;
	}
}

enum LDR_STR_SE_HW_OPCODE {
	TB_SE_STRSH = 0,
	TB_SE_LDSB,
	TB_SE_LDRH,
	TB_SE_LDSH
};

//THUMB.08
// LDR/STR with Sign-extension and halfword/byte transfer
void Arm7tdmi::TB_LDR_STR_SE_HW(u16 op) {
	u32 opp = op;
	u32 rnVal = rReg(RB(op));
	u32 rmVal = rReg((op >> 6) & 0x7);
	u8 rd = RD_LOW(op);
	u32 address = rnVal + rmVal;
	u32 data = 0;
	switch ((op & 0x0C00) >> 10) {
	case TB_SE_STRSH:
		bus->write16(address, rReg(rd));
		break;
	case TB_SE_LDSB:
		wReg(rd, (s32)(s8)bus->read8(address));
		break;
	case TB_SE_LDRH:
		wReg(rd, (u16)bus->read16(address));
		break;
	case TB_SE_LDSH:
		data = bus->read16(address);
		wReg(rd, (s32)(s16)data);
		break;
	}
}

enum LDR_STR_IMMEDIATE_OPCODE {
	TB_STR_IMM =0,
	TB_LDR_IMM,
	TB_STRB_IMM,
	TB_LDRB_IMM
};
//THUMB.09
//immediate offset
void Arm7tdmi::TB_LDR_STR_IMMEDIATE(u16 op) {
	u32 rnVal = rReg((op >> 3) & 0x7);
	u8 rd = RD_LOW(op);
	u32 address = 0;
	u8 immed5 = (((op >> 6) & 0x1F));
	switch ((op >> 11) & 0x3) {
	case TB_STR_IMM:
		address = rnVal + immed5 * 4;
		bus->write32(address, rReg(rd));
		break;
	case TB_LDR_IMM:
		address = rnVal + immed5 * 4;
		wReg(rd, bus->read32(address));
		break;
	case TB_STRB_IMM:
		address = rnVal + immed5;
		bus->write8(address, (u8)rReg(rd));
		break;
	case TB_LDRB_IMM:
		address = rnVal + immed5;
		wReg(rd, bus->read8(address));
		break;
	default:
		break;
	}
}

//THUMB.10
void Arm7tdmi::TB_LDRH_STRH(u16 op) {
	u32 addr = rReg(RB(op)) + (NN(op) << 1);
	if (op & BIT(11)) { //LDRH
		u16 data = bus->read16(addr);
		wReg(RD_LOW(op), data);
	}
	else { //STRH
		u16 data = rReg(RD_LOW(op));
		bus->write16(addr, data);
	}
}


//THUMB.11: load / store SP - relative (LDR4/STR3)
void Arm7tdmi::TB_LDRSP_STRSP(u16 op) {
	if (op & BIT(11)) { //LDR
		u32 data = bus->read32(rReg(13) + (op & 0xFF));
		wReg(RD_HIGH(op), data);
	}
	else { //STR
		u32 data = rReg(RD_HIGH(op));
		bus->write32(rReg(13) + (op & 0xFF), data);
	}
}

//THUMB.15
void Arm7tdmi::TB_LDMIA_STMIA(u16 op) {
	u8 rn = RN(op);
	u32 rnVal = rReg(rn);
	u32 address = rnVal;
	if (op & BIT(11)) { // LDMIA
		for (int i = 0; i < 8; i++) {
			if (op & BIT(i)) {
				wReg(i, bus->read32(address));
				address += 4;
			}
		}
	}
	else {
		for (int i = 0; i < 8; i++) {
			if (op & BIT(i)) {
				bus->write32(address, rReg(i));
				address += 4;
			}
		}
	}
	wReg(rn, rnVal + (countSetBits(op & 0xFF) * 4));
}

//THUMB.12
void Arm7tdmi::TB_GET_REL_ADDR(u16 op) {
	u8 offset = op & 0xFF;
	u8 rd = RD_HIGH(op);
	if (!(op & BIT(11))) { // ADD (5)
		wReg(rd, (r[15] & 0xFFFFFFFC) + (offset << 2));
	}
	else { // ADD(6)
		wReg(rd, rReg(13) + (offset << 2));
	}
	
}

//THUMB.13
void Arm7tdmi::TB_ADD_OFFSET_SP(u16 op) {
	u16 offset = (op & 0x7F) << 2;
	if (op & BIT(7)) { // SUB
		wReg(13, rReg(13) - offset);
	}
	else { // ADD
		wReg(13, rReg(13) + offset);
	}
}

//THUMB.14
void Arm7tdmi::TB_PUSH(u16 op) {
	u32 startAddress = rReg(13) - (countSetBits(op & 0x1FF) * 4);
	u32 address = startAddress;
	for (int i = 0; i < 8; i++) {
		if (op & BIT(i)) {
			bus->write32(address, rReg(i));
			address += 4;
		}
	}
	if (op & BIT(8)) {//BIT R
		bus->write32(address, rReg(14));
		address += 4;
	}
	wReg(13, startAddress);
}

//THUMB.14
void Arm7tdmi::TB_POP(u16 op) {
	u32 address = rReg(13);

	for (int i = 0; i < 8; i++) {
		if (op & BIT(i)) {
			wReg(i, bus->read32(address));
			address += 4;
		}
	}

	if (op & BIT(8)) {//BIT R
		u32 value = bus->read32(address);
		wReg(15, (value & 0xFFFFFFFE)+2);
		address += 4;

	}
	wReg(13, address);
}