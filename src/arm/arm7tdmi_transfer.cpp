
#include "arm/arm7tdmi.h"

#define RN(op) ((op >> 16) & 0xF)
#define RD(op) ((op >> 12) & 0xF)
#define RM(op) (op & 0xF)
#define BIT_W(op) (op & BIT(21))
#define BIT_U(op) (op & BIT(23))
#define BIT_P(op) (op & BIT(24))
#define BIT_B(op) (op & BIT(22))
#define EVAL_CONDITION true

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
#define IA(sa, op)		sa = rReg(RN(op));

// increment before
#define IB(sa, op)		sa = rReg(RN(op)) + 4;

//decrement after
#define DA(sa, op)		sa = rReg(RN(op)) - (setBits * 4) + 4;

//decrement before
#define DB(sa, op)		sa = rReg(RN(op)) - (setBits * 4);

static u8 countSetBits(u32 n) {
	n = n & 0xFFFF; //16 least significant bits
	u8 count = 0;
	while (n) {
		n &= (n - 1);
		count++;
	}
	return count;
}

void Arm7tdmi::SWP(u32 op) {
	u32 adress = rReg(RN(op));
	u32 tmp = bus->read32(adress);
	u32 final = tmp >> (8 * (adress & 0x3)) | tmp << (32 - (8 * (adress & 0x3)));
	bus->write32(adress, rReg(RM(op)));
	wReg(RD(op), final);
}

void Arm7tdmi::SWPB(u32 op) {
	u32 adress = rReg(RN(op));
	u8 tmp = bus->read8(adress);
	bus->write8(adress, rReg(RM(op)));
	wReg(RD(op), tmp);
}

void Arm7tdmi::LDM(u32 op) {
	u8 setBits = countSetBits(op);
	u32 start_adress;
	bool allowWriteback = true;
	switch ((op & (BIT(24) | BIT(23))) >> 23) {
	case 0: DA(start_adress, op); break;
	case 1: IA(start_adress, op); break;
	case 2: DB(start_adress, op); break;
	case 3: default: IB(start_adress, op); break;
	}
	u32 adress = start_adress;
	if (!(op & BIT(22))) { // LDM (1) 
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				wReg(i, bus->read32(adress));
				adress += 4;
				if (RN(op) == i && BIT_W(op)) {
					allowWriteback = false;
				}
			}
		}
		if (op & BIT(15)) {
			u32 value = bus->read32(adress);
			wReg(15, (value & 0xFFFFFFFC));
		}
	}
	else if (op & BIT(15)) { // LDM (3)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				wReg(i, bus->read32(adress));
				adress += 4;
				if (RN(op) == i && BIT_W(op)) {
					allowWriteback = false;
				}
			}
		}
		if (currentModeHasSPSR()) {
			cpsr = getSPSRValue();
		}
		u32 value = bus->read32(adress);
		wReg(15, value);

	}
	else { // LDM (2)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				wRegMode(i, bus->read32(adress), ARM7TDMI_MODE_USER);
				adress += 4;
				if (RN(op) == i && BIT_W(op)) {
					allowWriteback = false;
				}
			}
		}
	}
	if (BIT_W(op) && allowWriteback) {
		if (BIT_U(op)) {
			wReg(RN(op), rReg(RN(op)) + (setBits * 4));
		}
		else {
			wReg(RN(op), rReg(RN(op)) - (setBits * 4));
		}
	}

	if (!setBits) {// weird behaviour impl.
		wReg(15, bus->read32(rReg(RN(op))));
		wReg(RN(op), rReg(RN(op)) + 0x40);
	}
}

u32 Arm7tdmi::getNewBase(u32 op) {
	if (BIT_U(op)) {
		return(rReg(RN(op)) + (countSetBits(op) * 4));
	}
	else {
		return (rReg(RN(op)) - (countSetBits(op) * 4));
	}
}

void Arm7tdmi::STM(u32 op) {
	u8 setBits = countSetBits(op);
	u32 start_adress = 0;
	u8 howManySoFar = 0;
	switch ((op & (BIT(24) | BIT(23))) >> 23) {
	case 0: DA(start_adress, op); break;
	case 1: IA(start_adress, op); break;
	case 2: DB(start_adress, op); break;
	case 3: default: IB(start_adress, op); break;
	}
	u32 adress = start_adress;
	if ((op & BIT(22)) == 0) {//STM (1)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				if ((howManySoFar != 0) && i == RN(op) && BIT_W(op)) {
					bus->write32(adress, getNewBase(op));
				}
				else {
					bus->write32(adress, rReg(i));
				}
				adress += 4;
				howManySoFar++;
			}
		}
	}
	else { //STM (2)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				if (howManySoFar != 0 && i == RN(op) && BIT_W(op)) {
					bus->write32(adress, getNewBase(op)); //UNCHECKED
				}
				else {
					bus->write32(adress, rRegMode(i, ARM7TDMI_MODE_USER));
				}
				adress += 4;
				howManySoFar++;
			}
		}
	}
	if (op & BIT(15)) {
		bus->write32(adress, rReg(15) + 4); //TODO - be sure of +4
		adress += 4;
	}
	if (BIT_W(op)) {
		wReg(RN(op), getNewBase(op));
	}

	if (!setBits) {// empty rList behaviour
		bus->write32(rReg(RN(op)), getPCInArmOrThumb());
		wReg(RN(op), rReg(RN(op)) + 0x40);
	}
}

enum SHIFT_TYPE {
	LSL = 0,
	LSR,
	ASR,
	ROR_OR_RRX
};

bool Arm7tdmi::getAddressMode2(u32& op, u32& adress, const u32& rnVal, u32* thingToWrite)
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
		default:
			break;
		}
	}

	if (BIT_P(op)) { // P and W set, pre-indexed addressing		
		if (BIT_U(op)) {
			adress = rnVal + finalOffset;
		}
		else {
			adress = rnVal - finalOffset;
		}
		if (BIT_W(op)) {
			*thingToWrite = adress;
			return true;
		}
	}
	else if (!BIT_W(op) && !BIT_P(op)) { //standard post-indexed addressing
		adress = rnVal;
		if (BIT_U(op)) { // U set
			*thingToWrite = rnVal + finalOffset;
		}
		else {
			*thingToWrite = rnVal - finalOffset;
		}
		return true;
	}
	else {  //privileged post-indexed addressing
		printf("T operation\n");
		exit(222);
	}
	return false;
}

void Arm7tdmi::LDR(u32 op) { //LDR { , T, B, BT} (mode 2 or mode 2 P)
	//N.B.
	// If LDRBT is executed when the processor is in a privileged mode, the memory system is signaled to treat
	// the access as if the processor were in User mode.
	u32 adress = 0;
	u32 rnVal = rReg(RN(op));

	u32 thingToWrite = 0;
	bool writeNeeded = getAddressMode2(op, adress, rnVal, &thingToWrite);

	if (writeNeeded) {
		wReg(RN(op), thingToWrite);
	}

	if (BIT_B(op)) { // LDRB
		wReg(RD(op), bus->read8(adress));
	}
	else { // LDR
		u32 data = bus->read32(adress);
		u32 final = data >> (8 * (adress & 0x3)) | data << (32 - (8 * (adress & 0x3)));

		if (RD(op) == 15) {
			wReg(15, final & 0xFFFFFFFC);
		}
		else {
			wReg(RD(op), final);
		}
	}
}

void Arm7tdmi::STR(u32 op) {
	u32 adress = 0;
	u32 rnVal = rReg(RN(op));
	u32 thingToWrite = 0;
	bool writeNeeded = getAddressMode2(op, adress, rnVal, &thingToWrite);
	u32 data;
	if (RD(op) == 15) {
		data = rReg(RD(op)) + 4;
	}
	else {
		data = rReg(RD(op));
	}

	if (writeNeeded) {
		wReg(RN(op), thingToWrite);
	}

	if (BIT_B(op)) {
		bus->write8(adress, (u8)data);
	}
	else {
		bus->write32(adress, data);
	}
}

enum LDR_STR_MODE3_OPCODE {
	OP_H = 0b1011,
	OP_SB = 0b1101,
	OP_SH = 0b1111
};

void Arm7tdmi::STR2(u32 op) { //STRH
	u32 adress = 0; // final adress to be deducted by the following flow
	u32 rnVal = rReg(RN(op));
	u32 offset = 0;
	if (op & BIT(22)) { // offset is immediate
		offset = (u8)(((op & 0xF00) >> 4) | (op & 0xF));
	}
	else {
		offset = rReg(RM(op));
	}

	if (BIT_P(op)) { // p and W set, pre indexed
		if (BIT_U(op)) { // U set
			adress = rnVal + offset;
		}
		else {
			adress = rnVal - offset;
		}
		if (BIT_W(op)) {
			bus->write16(adress, rReg(RD(op)));
			wReg(RN(op), adress);
			return;
		}
	}

	else if (!BIT_W(op) && !BIT_P(op)) { // post indexed
		adress = rnVal;
	}

	bus->write16(adress, rReg(RD(op)));

	if (!BIT_W(op) && !BIT_P(op)) {
		if (BIT_U(op)) { // U set

			wReg(RN(op), rnVal + offset);
		}
		else {
			wReg(RN(op), rnVal - offset);
		}
	}
}

void Arm7tdmi::LDR2(u32 op) {//LDRSB, LDRH, LDRSH
	u32 adress = 0; // final adress to be deducted by the following flow
	u32 rnVal = rReg(RN(op));
	u32 offset = 0;

	if (op & BIT(22)) { // offset is immediate
		offset = (u8)(((op & 0xF00) >> 4) | (op & 0xF));
	}
	else {
		offset = rReg(RM(op));
	}
	if (BIT_P(op)) {
		if (BIT_U(op)) { // U set
			adress = rnVal + offset;
		}
		else {
			adress = rnVal - offset;
		}
		if (BIT_W(op)) {// p and W set, pre indexed
			wReg(RN(op), adress);
		}
	}
	else if (!BIT_W(op)) { // post indexed
		adress = rnVal;
		if (BIT_U(op)) { // U set
			wReg(RN(op), rnVal + offset);
		}
		else {
			wReg(RN(op), rnVal - offset);
		}
	}

	switch ((op >> 4) & 0xF) {
	case OP_H:
		if (adress & 0x1) {
			u32 data = bus->read16(adress - 1);
			//ROR 8 data
			wReg(RD(op), data >> 8 | data << 24);// LDRH
		}
		else {
			wReg(RD(op), bus->read16(adress));// LDRH
		}
		break;
	case OP_SB:
		wReg(RD(op), (s32)(s8)bus->read8(adress));// LDRSB
		break;
	case OP_SH:
		if (adress & 0x1) {
			wReg(RD(op), (s32)(s8)bus->read8(adress));// LDRSB
		}
		else {
			wReg(RD(op), (s32)(s16)bus->read16(adress));// LDRSH
		}
		break;
	default:
		printf("Invalid LDR2 opcode\n");
		exit(1);
	}
}
