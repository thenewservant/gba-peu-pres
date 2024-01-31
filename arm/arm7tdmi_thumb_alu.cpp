#include "arm7tdmi.h"

/*
arm7tdmi_thumb_alu.cpp: alu operations, move shifted register,
add/subtract, add/subtract with carry, compare, logical operations...
*/
#define RD(op)			(op & 0x7)
#define RS(op)			((op >> 3) & 0x7)
#define MOVE_OPCODE(op) ((op >>11) & 0x3)

//should be used only after assignments are done
#define CHECKCPSR_NZ cpsr = (cpsr & ~N) | ((result & BIT(31)) ? N : 0); \
					 cpsr = (cpsr & ~Z) | ((result == 0) ? Z : 0)


void Arm7tdmi::TB_MOV(u16 op) {
	u8 rd = RD(op);
	u8 rs = RS(op);
	r[rd] = r[rs];
	//CHECKCPSR_NZ;
}

//void Arm7tdmi::TB_AND(u16 op) {
//	u8 rd = REG_20(op);
//	u8 rm = REG_53(op);
//	r[rd] = r[rd] & r[rm];
//	cpsr = (cpsr & ~N) | ((r[rd] & BIT(31)) ? N : 0);
//	cpsr = (cpsr & ~Z) | ((r[rd] == 0) ? Z : 0);
//}
