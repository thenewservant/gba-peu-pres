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
		TB_LDRSP_STRSP(op);
	}
	else if (IS_TB_LOAD_N_STORE_HALFWORD(op)) {
		TB_LDRH_STRH(op);
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
		TB_ALU_OP(op);
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