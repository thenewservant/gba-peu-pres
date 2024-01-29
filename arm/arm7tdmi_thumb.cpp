#include "arm7tdmi.h"
#define REG_53(x) ((x>>3) & 0x7) // register pointed from bits 5 to 3
#define REG_20(x) (x & 0x7) // register pointed from bits 2 to 0
#define REG_10_8(x) ((x>>8) & 0x7) // register pointed from bits 10 to 8

#define CHECKCPSR cpsr = (cpsr & ~N) | ((r[rd] & BIT(31)) ? N : 0); \
				  cpsr = (cpsr & ~Z) | ((r[rd] == 0) ? Z : 0)

// THUMB operations

void Arm7tdmi::evaluateThumb(u16 op) {
	if ((op & 0xFC00) == 0x4000) {
		// THUMB Alu Operation
	}
	else{//  ((op & 0xF000) == 0xD000) { 
		thumbCondBranch(op);
	}
}

void Arm7tdmi :: thumbCondBranch(u16 op) {
	cpsr = 0;
}

void Arm7tdmi::TB_MOV1(u16 op) {
	u8 rd = REG_10_8(op);
	u8 imm = op & 0xFF;
	r[rd] = imm;
	cpsr = (cpsr & ~N) | ((imm & BIT(31)) ? N : 0);
	cpsr = (cpsr & ~Z) | ((imm == 0) ? Z : 0);
}


void Arm7tdmi::TB_ROR(u16 op) {
	u8 rsVal = r[REG_53(op)];
	u8 rd = REG_20(op);
	u32 rdVal = r[rd];
	u8 rs4to0 = rsVal & 0x1F; // Rs[4:0]
	if (rsVal == 0) {

	}
	else if (rs4to0 == 0) {
		cpsr = (cpsr & ~C) | ((rdVal & BIT(31)) ? C : 0);
	}
	else {
		cpsr = (cpsr & ~C) | ((rdVal & BIT(rs4to0 - 1)) ? C : 0);
		rdVal = (rdVal >> rs4to0) | (rdVal << (32 - rs4to0));
		r[rd] = rdVal;
		cpsr = (cpsr & ~N) | ((r[rd] & BIT(31)) ? N : 0);
		cpsr = (cpsr & ~Z) | ((r[rd] == 0) ? Z : 0);
	}
}

void Arm7tdmi::TB_AND(u16 op) {
	u8 rd = REG_20(op);
	u8 rm = REG_53(op);
	r[rd] = r[rd] & r[rm];
	cpsr = (cpsr & ~N) | ((r[rd] & BIT(31)) ? N : 0);
	cpsr = (cpsr & ~Z) | ((r[rd] == 0) ? Z : 0);
}

/**
Rd = Rd EOR Rm
N Flag = Rd[31]
Z Flag = if Rd == 0 then 1 else 0
*/
void Arm7tdmi::TB_EOR(u16 op) {
	u8 rd = REG_20(op);
	u8 rm = REG_53(op);
	r[rd] = r[rd] ^ r[rm];
	cpsr = (cpsr & ~N) | ((r[rd] & BIT(31)) ? N : 0);
	cpsr = (cpsr & ~Z) | ((r[rd] == 0) ? Z : 0);
}

void Arm7tdmi::TB_ORR(u16 op) {
	u8 rd = REG_20(op);
	u8 rm = REG_53(op);
	r[rd] = r[rd] | r[rm];
	cpsr = (cpsr & ~N) | ((r[rd] & BIT(31)) ? N : 0);
	cpsr = (cpsr & ~Z) | ((r[rd] == 0) ? Z : 0);
}

//Rd = Rd AND NOT Rm
//N Flag = Rd[31]
//Z Flag = if Rd == 0 then 1 else 0
void Arm7tdmi::TB_BIC(u16 op) {
	u8 rd = REG_20(op);
	u8 rm = REG_53(op);
	r[rd] = r[rd] & ~r[rm];
	cpsr = (cpsr & ~N) | ((r[rd] & BIT(31)) ? N : 0);
	cpsr = (cpsr & ~Z) | ((r[rd] == 0) ? Z : 0);
}



void Arm7tdmi::TB_MVN(u16 op) {
	u8 rd = REG_20(op);
	u8 rm = REG_53(op);
	r[rd] = ~r[rm];
	CHECKCPSR;
}

void Arm7tdmi::TB_TST(u16 op) {
	u8 rn = REG_20(op);
	u8 rm = REG_53(op);
	u8 alu_out = r[rn] & r[rm];
	cpsr = (cpsr & ~N) | ((alu_out & BIT(31)) ? N : 0);
	cpsr = (cpsr & ~Z) | ((alu_out == 0) ? Z : 0);
}