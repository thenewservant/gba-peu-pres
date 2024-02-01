#include "arm7tdmi.h"


#define RD(op) (op & 0x7)
#define RB(op) ((op >> 3) & 0x7)
#define NN(op) ((op >> 6) & 0x1F) // for thumb 10 only


//THUMB.10
void Arm7tdmi::TB_LDRH_STRH(u16 op) {
	u32 addr = rReg(RB(op)) + (NN(op) << 1);
	if (op & BIT(11)) { //LDRH
		u16 data = bus->read16(addr);
		wReg(RD(op), data);
	}
	else { //STRH
		u16 data = rReg(RD(op));
		bus->write16(addr, data);
	}
}
//THUMB.10

//THUMB.11: load / store SP - relative
void Arm7tdmi::TB_LDRSP_STRSP(u16 op) {
	if (op & BIT(11)) { //LDR
		u32 data = bus->read32(rReg(13) + (op & 0xFF));
		wReg(RD(op), data);
	}
	else { //STR
		u32 data = rReg(RD(op));
		bus->write32(rReg(13) + (op & 0xFF), data);
	}
}