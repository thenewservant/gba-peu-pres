#include "arm7tdmi.h"
#define REG_53(x) ((x>>3) & 0x7) // register pointed from bits 5 to 3
#define REG_20(x) (x & 0x7) // register pointed from bits 2 to 0
#define REG_10_8(x) ((x>>8) & 0x7) // register pointed from bits 10 to 8

#define CHECKCPSR cpsr = (cpsr & ~N) | ((r[rd] & BIT(31)) ? N : 0); \
				  cpsr = (cpsr & ~Z) | ((r[rd] == 0) ? Z : 0)


#define IS_TB_SOFTWARE_INTERRUPT(op)	         ((op & 0xFF00) == 0xDF00)
#define IS_TB_COND_BRANCH(op)			         ((op & 0xF000) == 0xD000)
#define IS_TB_UNCOND_BRANCH(op)			         ((op & 0xF800) == 0xE000)
#define IS_TB_LONG_BRANCH_WITH_LINK(op)	         ((op & 0xF000) == 0xF000)
#define IS_TB_MULTIPLE_LOAD_N_STORE(op)          ((op & 0xF000) == 0xC000)
#define IS_TB_PUSH_N_POP_REGISTERS(op)           ((op & 0xF600) == 0xB400)
#define IS_TB_ADD_OFFSET_TO_STACK_POINTER(op)    ((op & 0xFF00) == 0xB000)
#define IS_TB_LOAD_ADDRESS(op)				     ((op & 0xF000) == 0xA000)
#define IS_TB_SP_RELATIVE_LOAD_N_STORE(op)	     ((op & 0xF000) == 0x9000)
#define IS_TB_LOAD_N_STORE_HALFWORD(op)          ((op & 0xF000) == 0x8000)
#define IS_TB_LOAD_N_STORE_WITH_IMM_OFFSET(op)   ((op & 0xE000) == 0x6000)
#define IS_TB_LOAD_N_STORE_S_E_BYTE_AND_HW(op)   ((op & 0xF200) == 0x5200)
#define IS_TB_LOAD_N_STORE_WITH_REL_OFFSET(op)   ((op & 0xF200) == 0x5000)
#define IS_TB_PC_RELATIVE_LOAD(op)			     ((op & 0xF800) == 0x4800)
#define IS_TB_HIGH_REG_OPERATIONS_AND_BRANCH(op) ((op & 0xFC00) == 0x4400)
#define IS_TB_ALU_OPERATION(op)				     ((op & 0xFC00) == 0x4000)
#define IS_TB_MOV_COMP_ADD_SUB_IMMEDIATE(op)     ((op & 0xE000) == 0x2000)
#define IS_TB_ADD_AND_SUBSTRACT(op)			     ((op & 0xFC00) == 0x1C00) // This check must happen BEFORE 
#define IS_TB_MOVE_SHIFTED_REGISTER(op)		     ((op & 0xE000) == 0x0000) // this one

void Arm7tdmi::evaluateThumb(u16 op) {
	if (IS_TB_SOFTWARE_INTERRUPT(op)) {

	}
	else if (IS_TB_COND_BRANCH(op)) {
		TB_COND_BRANCH(op);
	}
	else if (IS_TB_UNCOND_BRANCH(op)) {
		TB_UNCOND_BRANCH(op);
	}
	else if (IS_TB_LONG_BRANCH_WITH_LINK(op)) {

	}
	else if (IS_TB_MULTIPLE_LOAD_N_STORE(op)) {

	}
	else if (IS_TB_PUSH_N_POP_REGISTERS(op)) {

	}
	else if (IS_TB_ADD_OFFSET_TO_STACK_POINTER(op)) {

	}
	else if (IS_TB_LOAD_ADDRESS(op)) {

	}
	else if (IS_TB_SP_RELATIVE_LOAD_N_STORE(op)) {

	}
	else if (IS_TB_LOAD_N_STORE_HALFWORD(op)) {

	}
	else if (IS_TB_LOAD_N_STORE_WITH_IMM_OFFSET(op)) {

	}
	else if (IS_TB_LOAD_N_STORE_S_E_BYTE_AND_HW(op)) {

	}
	else if (IS_TB_LOAD_N_STORE_WITH_REL_OFFSET(op)) {

	}
	else if (IS_TB_PC_RELATIVE_LOAD(op)) {

	}
	else if (IS_TB_HIGH_REG_OPERATIONS_AND_BRANCH(op)) {

	}
	else if (IS_TB_ALU_OPERATION(op)) {
		//thumbALUOperation(op);
	}
	else if (IS_TB_MOV_COMP_ADD_SUB_IMMEDIATE(op)) {

	}
	else if (IS_TB_ADD_AND_SUBSTRACT(op)) {

	}
	else if (IS_TB_MOVE_SHIFTED_REGISTER(op)) {

	}
	else {
		printf("Unknown Thumb instruction: %04X\n", op);
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