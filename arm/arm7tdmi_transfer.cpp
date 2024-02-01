#include "arm7tdmi.h"



#define RN(op) ((op >> 16) & 0xF)
#define RD(op) ((op >> 12) & 0xF)
#define RM(op) (op & 0xF)
#define BIT_W(op) (op & BIT(21))
#define BIT_U(op) (op & BIT(23))

//to be used ONLY to distinguish LDRB(T) from LDR(B) 
// Reference : Arm Architecture Ref Manual, A5.2.9
#define IS_BT_OR_T(op) ((op & (BIT(24) | BIT(21))) == (BIT(21)))

//
//address = Rn
//
//if U == 1 then
//	Rn = Rn + Rm
//else /* U == 0 */
//	Rn = Rn - Rm

#define BT_T_POST_IDX_ADRESSING(sa, op)   sa= rReg(RN(op));\
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

	if (IS_BT_OR_T(op)) {
		u32 address;
		BT_T_POST_IDX_ADRESSING(address, op);
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
//
//Opcode Format
//
//Bit    Expl.
//31 - 28  Condition(Must be 1111b for PLD)
//27 - 26  Must be 01b for this instruction
//25     I - Immediate Offset Flag(0 = Immediate, 1 = Shifted Register)
//24     P - Pre / Post(0 = post; add offset after transfer, 1 = pre; before trans.)
//23     U - Up / Down Bit(0 = down; subtract offset from base, 1 = up; add to base)
//22     B - Byte / Word bit(0 = transfer 32bit / word, 1 = transfer 8bit / byte)
//When above Bit 24 P = 0 (Post - indexing, write - back is ALWAYS enabled) :
//	21     T - Memory Management(0 = Normal, 1 = Force non - privileged access)
//	When above Bit 24 P = 1 (Pre - indexing, write - back is optional) :
//	21     W - Write - back bit(0 = no write - back, 1 = write address into base)
//	20     L - Load / Store bit(0 = Store to memory, 1 = Load from memory)
//	0 : STR{ cond }{B} {T} Rd, <Address>; [Rn + / -<offset>] = Rd
//	1: LDR{ cond }{B} {T} Rd, <Address>; Rd = [Rn + / -<offset>]
//	(1: PLD <Address>; Prepare Cache for Load, see notes below)
//	Whereas, B = Byte, T = Force User Mode(only for POST - Indexing)
//	19 - 16  Rn - Base register               (R0..R15) (including R15 = PC + 8)
//	15 - 12  Rd - Source / Destination Register(R0..R15) (including R15 = PC + 12)
//	When above I = 0 (Immediate as Offset)
//	11 - 0   Unsigned 12bit Immediate Offset(0 - 4095, steps of 1)
//	When above I = 1 (Register shifted by Immediate as Offset)
//	11 - 7   Is - Shift amount(1 - 31, 0 = Special / See below)
//	6 - 5    Shift Type(0 = LSL, 1 = LSR, 2 = ASR, 3 = ROR)
//	4      Must be 0 (Reserved, see The Undefined Instruction)
//	3 - 0    Rm - Offset Register(R0..R14) (not including PC = R15)


void Arm7tdmi::STR(u32 op) {
	if (IS_BT_OR_T(op)) {
		u32 address;
		BT_T_POST_IDX_ADRESSING(address, op);
		if (op & BIT(22)) { // STRBT
			bus->write8(address, rReg(RD(op)));
			wReg(RN(op), address);
		}
		else { // STRT

		}
	}
	else { // STR, STRB
		if (op & BIT(22)) { // STRB

		}
		else { // STR

		}
	}
}

//mode 3 access:

u32 mode3address(u32 op) {

	return 0;
}

void Arm7tdmi::STR2(u32 op) {

}

void Arm7tdmi::LDR2(u32 op) {//LDRSB, LDRH, LDRSH and STRH...

}