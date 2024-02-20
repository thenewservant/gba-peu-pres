#include "arm7tdmi.h"



#define RN(op) ((op >> 16) & 0xF)
#define RD(op) ((op >> 12) & 0xF)
#define RM(op) (op & 0xF)
#define BIT_W(op) (op & BIT(21))
#define BIT_U(op) (op & BIT(23))

//to be used ONLY to distinguish LDRB(T) from LDR(B) 
// Reference : Arm Architecture Ref Manual, A5.2.9
#define IS_BT_OR_T(op) ((op & (BIT(24) | BIT(21))) == (BIT(21)))

#define BT_T_POST_IDX_ADRESSING(sa, op)   sa= rReg(RN(op));\
											if (BIT_U(op)){\
												wReg(RN(op), sa + rReg(RM(op)));\
											} else {\
												wReg(RN(op), sa - rReg(RM(op)));\
											}

//increment after
#define IA(sa, op)		sa = rReg(RN(op));\
						if (BIT_W(op)){\
								wReg(RN(op), sa + (countSetBits(op&0xFFFF) * 4));\
						}

// increment before
#define IB(sa, op)		sa = rReg(RN(op)) + 4;\
						if (BIT_W(op)){\
								wReg(RN(op), sa - 4 + (countSetBits(op&0xFFFF) * 4));\
						}

//decrement after
#define DA(sa, op)		sa = rReg(RN(op)) - (countSetBits(op) * 4) + 4;\
						if (BIT_W(op)){\
								wReg(RN(op), rReg(RN(op)) - (countSetBits(op&0xFFFF) * 4));\
						}

//decrement before
#define DB(sa, op)		sa = rReg(RN(op)) - (countSetBits(op) * 4);\
						if (BIT_W(op)){\
								wReg(RN(op), rReg(RN(op)) - (countSetBits(op&0xFFFF) * 4));\
						}

static u8 countSetBits(u32 n) {
	n = n & 0xFFFF; //16 least significant bits
	u8 count = 0;
	while (n) {
		n &= (n - 1);
		count++;
	}
	return count;
}

//UNFINISHED
void Arm7tdmi::SWP(u32 op) {
	u32 address = rReg(RN(op));
	u32 tmp = bus->read32(address);
	bus->write32(address, rReg(RM(op)));
	wReg(RD(op), tmp);
}

void Arm7tdmi::SWPB(u32 op) {
	u32 address = rReg(RN(op));
	u8 tmp = bus->read8(address);
	bus->write8(address, rReg(RM(op)));
	wReg(RD(op), tmp);
}

void Arm7tdmi::LDM(u32 op) {
	u32 start_adress;
	u32 end_adress = 0;
	switch ((op & (BIT(24) | BIT(23))) >> 23) {
	case 0: DA(start_adress, op); break;
	case 1: IA(start_adress, op); break;
	case 2: DB(start_adress, op); break;
	case 3: default: IB(start_adress, op); break;
	}

	if (!(op & BIT(22))) { // LDM (1) 
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				wReg(i, bus->read32(start_adress));
				start_adress += 4;
			}
		}
		if (op & BIT(15)) {
			u32 value = bus->read32(start_adress);
			wReg(15, (value & 0xFFFFFFFC));
		}
	}
	else if (op & BIT(15)) { // LDM (3)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				wReg(i, bus->read32(start_adress));
				start_adress += 4;
			}
		}
		if (currentModeHasSPSR()) {
			cpsr = getSPSRValue();
		}
		u32 value = bus->read32(start_adress);
		wReg(15, value);
	}
	else { // LDM (2)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				wReg(i, bus->read32(start_adress));
				start_adress += 4;
			}
		}
	}
}

void Arm7tdmi::STM(u32 op) {
	u32 start_adress = 0;
	u32 end_adress = 0;
	switch ((op & (BIT(24) | BIT(23))) >> 23) {
	case 0: DA(start_adress, op); break;
	case 1: IA(start_adress, op); break;
	case 2: DB(start_adress, op); break;
	case 3: default: IB(start_adress, op); break;
	}
	if ((op & BIT(22)) == 0) {//STM (1)
		for (u8 i = 0; i <= 15; i++) {
			if (op & BIT(i)) {
				bus->write32(start_adress, rReg(i));
				start_adress += 4;
			}
		}
	}
	else { //STM (2)
		for (u8 i = 0; i <= 15; i++) {
			if (op & BIT(i)) {
				bus->write32(start_adress, rRegMode(i, ARM7TDMI_MODE_USER));
				start_adress += 4;
			}
		}
	}
}

#define BIT_P(op) (op & BIT(24))
#define BIT_U(op) (op & BIT(23))
#define BIT_B(op) (op & BIT(22))
#define EVAL_CONDITION true

enum SHIFT_TYPE {
	LSL = 0,
	LSR,
	ASR,
	ROR_OR_RRX
};

void Arm7tdmi::getAddressMode2(u32& op, u32& address, const u32& rnVal, const u32& offset)
{
	u32 finalOffset = 0;
	if (!(op & BIT(25))) { // offset is immediate
		finalOffset = (op & 0xFFF);
	}
	else if (!(op & 0xFF0)) { // register offset 
		finalOffset = rReg(RM(op));
	}
	else {//scaled register offset (with shifts)
		u32 shiftAmount = (op & 0xF80) >> 7;
		switch ((op & 0x60) >> 5) {
		case LSL: finalOffset = rReg(RM(op)) << (shiftAmount); break;
		case LSR: finalOffset = (shiftAmount) ? (rReg(RM(op)) >> (shiftAmount)) : 0; break;
		case ASR:
			if (shiftAmount) {
				finalOffset = rReg(RM(op)) >> (shiftAmount);
			}
			else {
				finalOffset = (rReg(RM(op)) & BIT(31)) ? 0xFFFFFFFF : 0;
			}
			break;
		case ROR_OR_RRX:
			if (shiftAmount) { // ROR
				finalOffset = (rReg(RM(op)) >> (shiftAmount)) | (rReg(RM(op)) << (32 - shiftAmount));
			}
			else {
				finalOffset = (rReg(RM(op)) >> 1) | ((cpsr & C) ? BIT(31) : 0);
			}
			break;
		}
	}

	if (BIT_P(op)) { // P and W set, pre indexed
		if (BIT_U(op)) {
			address = rnVal + finalOffset;
		}
		else {
			address = rnVal - finalOffset;
		}
		if (BIT_W(op)) {
			wReg(RN(op), address);
		}
	}
	else if (!BIT_W(op) && !BIT_P(op)) {
		address = rnVal;
		if (BIT_U(op)) { // U set
			wReg(RN(op), rnVal + finalOffset);
		}
		else {
			wReg(RN(op), rnVal - finalOffset);
		}
	}
	else {
		exit(222);
	}
}

void Arm7tdmi::LDR(u32 op) { //LDR { , T, B, BT} (mode 2 or mode 2 P)
	//N.B.
	// If LDRBT is executed when the processor is in a privileged mode, the memory system is signaled to treat
	// the access as if the processor were in User mode.
	u32 address = 0; // final address to be deducted by the following flow
	u32 rnVal = rReg(RN(op));
	u32 baseOffset = 0;

	getAddressMode2(op, address, rnVal, baseOffset);

	if (BIT_B(op)) { // LDRB
		wReg(RD(op), bus->read8(address));
	}
	else { // LDR
		u32 data = bus->read32(address);
		//wReg(RD(op), (data >> (8 * (address & 0x3))) | (data << (32 - (8 * (address & 0x3)))));
		wReg(RD(op), data);
		if (RD(op) == 15) {
			exit(55);
		}
	}
}

void Arm7tdmi::STR(u32 op) {
	u32 address = 0; // final address to be deducted by the following flow
	u32 rnVal = rReg(RN(op));
	u32 offset = 0;
	getAddressMode2(op, address, rnVal, offset);

	if (BIT_B(op)) {
		bus->write8(address, rReg(RD(op)));
	}
	else {
		bus->write32(address, rReg(RD(op)));
	}
}

void Arm7tdmi::STR2(u32 op) { //STRH
	u32 address = 0; // final address to be deducted by the following flow
	u32 rnVal = rReg(RN(op));
	u32 offset = 0;
	if (op & BIT(22)) { // offset is immediate
		offset = (u8)(((op & 0xF00) >> 4) | (op & 0xF));
	}
	else {
		offset = rReg(RM(op));
	}

	if (BIT_W(op) && BIT_P(op)) { // p and W set, pre indexed
		if (BIT_U(op)) { // U set
			address = rnVal + offset;
		}
		else {
			address = rnVal - offset;
		}
		wReg(RN(op), address);
	}
	else if (!BIT_W(op) && BIT_P(op)) { // offset
		if (BIT_U(op)) { // U set
			address = rnVal + offset;
		}
		else {
			address = rnVal - offset;
		}
	}
	else if (!BIT_W(op) && !BIT_P(op)) { // post indexed
		address = rnVal;
		if (BIT_U(op)) { // U set
			address = rnVal + offset;
		}
		else {
			address = rnVal - offset;
		}
		wReg(RN(op), address);
	}

	bus->write16(address, rReg(RD(op)));
}

void Arm7tdmi::LDR2(u32 op) {//LDRSB, LDRH, LDRSH
	u32 address = 0; // final address to be deducted by the following flow
	u32 rnVal = rReg(RN(op));
	u32 offset = 0;
	if (op & BIT(22)) { // offset is immediate
		offset = (u8)(((op & 0xF00) >> 4) | (op & 0xF));
	}
	else {
		offset = rReg(RM(op));
	}

	if (BIT_W(op) && BIT_P(op)) { // p and W set, pre indexed
		if (BIT_U(op)) { // U set
			address = rnVal + offset;
		}
		else {
			address = rnVal - offset;
		}
		if (EVAL_CONDITION) {
			wReg(RN(op), address);
		}
	}
	else if (!BIT_W(op) && BIT_P(op)) { // offset
		if (BIT_U(op)) { // U set
			address = rnVal + offset;
		}
		else {
			address = rnVal - offset;
		}
	}
	else if (!BIT_W(op) && !BIT_P(op)) { // post indexed
		address = rnVal;

		if (BIT_U(op)) { // U set
			address = rnVal + offset;
		}
		else {
			address = rnVal - offset;
		}
		wReg(RN(op), address);
	}

	if (op & BIT(22)) { // LDRH
		wReg(RD(op), bus->read16(address));
	}
	else { // LDRSH
		wReg(RD(op), bus->read16(address));
	}
}