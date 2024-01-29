#include "arm7tdmi.h"

/*
 void LDM(u32 op);
void STM(u32 op);

void LDR(u32 op);
void STR(u32 op);

void STR2(u32 op);
void LDR2(u32 op);

void SWP(u32 op);
void SWPB(u32 op);
*/

#define RN(op) ((op >> 16) & 0xF)
#define RD(op) ((op >> 12) & 0xF)
#define RM(op) (op & 0xF)
#define BIT_W(op) (op & BIT(21))

//increment after
// 
//start_address = Rn
//end_address = Rn + (Number_Of_Set_Bits_In(register_list) * 4) - 4
//if ConditionPassed(cond) and W == 1 then
//	Rn = Rn + (Number_Of_Set_Bits_In(register_list) * 4)

// increment before
// 
//start_address = Rn + 4
//end_address = Rn + (Number_Of_Set_Bits_In(register_list) * 4)
//if ConditionPassed(cond) and W == 1 then
//	Rn = Rn + (Number_Of_Set_Bits_In(register_list) * 4)
//

//decrement after
// 
//start_address = Rn - (Number_Of_Set_Bits_In(register_list) * 4) + 4
//end_address = Rn
//if ConditionPassed(cond) and W == 1 then
//	Rn = Rn - (Number_Of_Set_Bits_In(register_list) * 4)

//decrement before
//
//start_address = Rn - (Number_Of_Set_Bits_In(register_list) * 4)
//end_address = Rn - 4
//if ConditionPassed(cond) and W == 1 then
//	Rn = Rn - (Number_Of_Set_Bits_In(register_list) * 4)




//UNFINISHED
void Arm7tdmi::SWP(u32 op) {
	u32 address = rReg(RN(op));
	u32 tmp = bus->read32(address);
	bus->write32(address, rReg(RM(op)));
	wReg(RD(op),tmp);
}


void Arm7tdmi::SWPB(u32 op) {
	u32 address = rReg(RN(op));
	u8 tmp = bus->read8(address);
	bus->write8(address, rReg(RM(op)));
	wReg(RD(op), tmp);
}

//all LDM dependancies will be merged 
void Arm7tdmi::LDM(u32 op) {
	if (!(op & BIT(22))) { // LDM (1) 

	}
	else if (op & BIT(15)) { // LDM (3)

	}
	else { // LDM (2)

	}
}

void Arm7tdmi::STM(u32 op) {

}

void Arm7tdmi::LDR(u32 op) {

}

void Arm7tdmi::STR(u32 op) {

}

void Arm7tdmi::STR2(u32 op) {

}

void Arm7tdmi::LDR2(u32 op) {

}