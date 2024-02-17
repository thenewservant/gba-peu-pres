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

//THUMB.06
void Arm7tdmi::TB_LDRPC(u16 op) {
	u32 address = (rReg(15) & 0xFFFFFFFC) + ((op & 0xFF) * 4);
	wReg(RD_HIGH(op), bus->read32(address));
}
//THUMB.07
void Arm7tdmi::TB_LDR_STR_RELATIVE(u16 op) {
	u32 rnVal = rReg(RN(op));
	u32 rmVal = rReg(RO(op));
	u32 address = rnVal + rmVal;

	if (op & BIT(11)) { //LDR
		u32 data = bus->read32(address);
		wReg(RD_LOW(op), data);
	}
	else { //STR
		u32 data = rReg(RD_LOW(op));
		bus->write32(address, data);
	}
}

enum LDR_STR_IMMEDIATE_OPCODE {
	TB_STR_IMM =0,
	TB_LDR_IMM,
	TB_STRB_IMM,
	TB_LDRB_IMM
};
//THUMB.09
void Arm7tdmi::TB_LDR_STR_IMMEDIATE(u16 op) {
	u8 rd = RD_LOW(op);
	u32 address = 0;
	u8 immed5 = (((op >> 6) & 0x1F));
	switch ((op >> 11) & 0x3) {
	case TB_STR_IMM:
		address = rReg(RN(op)) + immed5 * 4;
		bus->write32(address, rReg(rd));
		break;
	case TB_LDR_IMM:
		address = rReg(RN(op)) + immed5 * 4;
		wReg(rd, bus->read32(address));
		break;
	case TB_STRB_IMM:
		address = rReg(RN(op)) + immed5;
		bus->write8(address, (u8)rReg(rd));
		break;
	case TB_LDRB_IMM:
		address = rReg(RN(op)) + immed5;
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

	wReg(rn, rn + (countSetBits(op) * 4));
}