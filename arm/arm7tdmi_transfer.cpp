#include "arm7tdmi.h"



#define RN(op) ((op >> 16) & 0xF)
#define RD(op) ((op >> 12) & 0xF)
#define RM(op) (op & 0xF)
#define BIT_W(op) (op & BIT(21))
#define BIT_U(op) (op & BIT(23))

//to be used ONLY to distinguish LDRB(T) from LDR(B) 
// Reference : Arm Architecture Ref Manual, A5.2.9
#define IS_LDRBT_OR_LDRT(op) ((op & (BIT(24) | BIT(21))) == (BIT(21)))

//
//address = Rn
//
//if U == 1 then
//	Rn = Rn + Rm
//else /* U == 0 */
//	Rn = Rn - Rm

#define LDRBT_LDRT_POST_IDX_ADRESSING(sa, op)   sa= rReg(RN(op));\
											if (BIT_U(op)){\
												wReg(RN(op), sa + rReg(RM(op)));\
											} else {\
												wReg(RN(op), sa - rReg(RM(op)));\
											}


//increment after
#define IA(sa, op)		sa = rReg(RN(op));\
						if (BIT_W(op)){\
								wReg(RN(op), sa + (countSetBits(op) * 4));\
						}

// increment before
#define IB(sa, op)		sa = rReg(RN(op)) + 4;\
						if (BIT_W(op)){\
								wReg(RN(op), sa - 4 + (countSetBits(op) * 4));\
						}

//decrement after
#define DA(sa, op)		sa = rReg(RN(op)) - (countSetBits(op) * 4) + 4;\
						if (BIT_W(op)){\
								wReg(RN(op), rReg(RN(op)) - (countSetBits(op) * 4));\
						}

//decrement before
#define DB(sa, op)		sa = rReg(RN(op)) - (countSetBits(op) * 4);\
						if (BIT_W(op)){\
								wReg(RN(op), rReg(RN(op)) - (countSetBits(op) * 4));\
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
	switch ((op & (BIT(24) | BIT(23))) >> 23) {
	case 0: DA(start_adress, op); break;
	case 1: IA(start_adress, op); break;
	case 2: DB(start_adress, op); break;
	case 3: default: IB(start_adress, op); break;;
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
			wReg(15, value & 0xFFFFFFFC);
		}
	}
	else if (op & BIT(15)) { // LDM (3)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				wReg(i, bus->read32(start_adress));
				start_adress += 4;
				if (currentModeHasSPSR()) {
					cpsr = getSPSRValue();
				}
				else {
					//UNPREDICTABLE
					u32 value = bus->read32(start_adress);
					wReg(15, value);
				}
			}
		}
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
	u32 start_adress;
	switch ((op & (BIT(24) | BIT(23))) >> 23) {
	case 0: DA(start_adress, op); break;
	case 1: IA(start_adress, op); break;
	case 2: DB(start_adress, op); break;
	case 3: default: IB(start_adress, op); break;
	}
	if (!(op & BIT(22))) {//STM (1)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				bus->write32(start_adress, rReg(i));
				start_adress += 4;
			}
		}
	}
	else { //STM (2)
		for (u8 i = 0; i < 15; i++) {
			if (op & BIT(i)) {
				bus->write32(start_adress, rRegMode(i, ARM7TDMI_MODE_USER));
				start_adress += 4;
			}
		}
	}
}

void Arm7tdmi::LDR(u32 op) { //LDR { , T, B, BT} 
	//N.B.
	// If LDRBT is executed when the processor is in a privileged mode, the memory system is signaled to treat
	// the access as if the processor were in User mode.

	if (IS_LDRBT_OR_LDRT(op)) {
		u32 address;
		LDRBT_LDRT_POST_IDX_ADRESSING(address, op);
		if (op & BIT(22)) { // LDRBT
			wReg(RD(op), bus->read8(address));
			wReg(RN(op), address);
		}
		else { // LDRT

		}
	}
	else { // LDR, LDRB
		if (op & BIT(22)) { // LDRB

		}
		else { // LDR

		}
	}
}

void Arm7tdmi::STR(u32 op) {

}

void Arm7tdmi::STR2(u32 op) {

}

void Arm7tdmi::LDR2(u32 op) {//LDRSB, LDRH, LDRSH

}