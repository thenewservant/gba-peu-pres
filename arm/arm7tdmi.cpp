#include "arm7tdmi.h"
#include <windows.h>
#include "tests/tests.h"

/* arm7tdmi.cpp
* General operation, instruction decoding, register access, etc.
*/


#define IS_BRANCH_AND_EXCHANGE(op)    ((op & 0x0FFFFFF0) == 0x012FFF10)
#define IS_SOFTWARE_INTERRUPT(op)     ((op & 0x0F000000) == 0x0F000000)
#define IS_BRANCH(op)                 ((op & 0x0E000000) == 0x0A000000)
#define IS_BLOCK_DATA_TRANSFER(op)    ((op & 0x0E000000) == 0x08000000)
#define IS_SINGLE_DATA_TRANSFER(op)   ((op & 0x0E000000) == 0x06000000)
#define IS_DATA_PROCESSING(op)        ((op & 0x0E000000) == 0x02000000)
#define IS_HALFWORD_DATA_TRANSFER(op) ((op & 0x0E000000) == 0x000000B0)
#define IS_SINGLE_DATA_SWAP(op)       ((op & 0x0FB00FF0) == 0x01000090)
#define IS_MULTIPLY(op)               ((op & 0x0F0000F0) == 0x00000090)
#define IS_MRS(op)                    ((op & 0x0FBF0FFF) == 0x010F0000)
#define IS_MSR_IMM(op)                ((op & 0x0FB0F000) == 0x0320F000)
#define IS_MSR_REG(op)                ((op & 0x0FB00FF0) == 0x01200000)


#define IS_LOAD_INSTRUCTION(op)				      (op & BIT(20))
#define IS_BYTE_TRANFER_INSTRUCTION(op)			  (op & BIT(22))
bool evalCondition(u32 cpsr, u32 op) {
	switch (op & 0xF0000000) {
	case ARM7TDMI_CONDITION_EQ:
		return FLAG(Z) == Z;
	case ARM7TDMI_CONDITION_NE:
		return FLAG(Z) == 0;
	case ARM7TDMI_CONDITION_CS:
		return FLAG(C) == C;
	case ARM7TDMI_CONDITION_CC:
		return FLAG(C) == 0;
	case ARM7TDMI_CONDITION_MI:
		return FLAG(N) == N;
	case ARM7TDMI_CONDITION_PL:
		return FLAG(N) == 0;
	case ARM7TDMI_CONDITION_VS:
		return FLAG(V) == V;
	case ARM7TDMI_CONDITION_VC:
		return FLAG(V) == 0;
	case ARM7TDMI_CONDITION_HI:
		return (FLAG(C) == C) && (FLAG(Z) == 0);
	case ARM7TDMI_CONDITION_LS:
		return (FLAG(C) == 0) || (FLAG(Z) == Z);
	case ARM7TDMI_CONDITION_GE:
		return (FLAG(N) == FLAG(V));
	case ARM7TDMI_CONDITION_LT:
		return (FLAG(N) != FLAG(V));
	case ARM7TDMI_CONDITION_GT:
		return (FLAG(Z) == 0) && (FLAG(N) == FLAG(V));
	case ARM7TDMI_CONDITION_LE:
		return (FLAG(Z) == Z) || (FLAG(N) != FLAG(V));
	case ARM7TDMI_CONDITION_AL:
		return true;
	case ARM7TDMI_CONDITION_NV:
	default:
		return false;
	}
}

u32 Arm7tdmi::rReg(u8 reg) {
	if (reg < 8) {
		return r[reg];
	}
	switch (CURRENT_MODE) {
	case ARM7TDMI_MODE_USER:
	case ARM7TDMI_MODE_SYS:
		return r[reg];
	case ARM7TDMI_MODE_FIQ:
		return (reg == 15) ? r[15] + 4 : rFiq[reg - 8];
	case ARM7TDMI_MODE_SVC:
		return ((reg == 13) || (reg == 14)) ? rSvc[reg - 13] : r[reg];
	case ARM7TDMI_MODE_ABT:
		return ((reg == 13) || (reg == 14)) ? rAbt[reg - 13] : r[reg];
	case ARM7TDMI_MODE_IRQ:
		return ((reg == 13) || (reg == 14)) ? rIrq[reg - 13] : r[reg];
	case ARM7TDMI_MODE_UND:
		return ((reg == 13) || (reg == 14)) ? rUnd[reg - 13] : r[reg];
	}
	return 0;
}

void Arm7tdmi::wReg(u8 reg, u32 value) {
	if (reg < 8) {
		r[reg] = value;
	}
	else {
		switch (CURRENT_MODE) {
		case ARM7TDMI_MODE_USER:
		case ARM7TDMI_MODE_SYS:
			r[reg];
		case ARM7TDMI_MODE_FIQ:
			(reg >= 15) ? r[15] = value : rFiq[reg - 13] = value;
		case ARM7TDMI_MODE_SVC:
			((reg == 13) || (reg == 14)) ? rSvc[reg - 13] = value : r[reg] = value;
		case ARM7TDMI_MODE_ABT:
			((reg == 13) || (reg == 14)) ? rAbt[reg - 13] = value : r[reg] = value;
		case ARM7TDMI_MODE_IRQ:
			((reg == 13) || (reg == 14)) ? rIrq[reg - 13] = value : r[reg] = value;
		case ARM7TDMI_MODE_UND:
			((reg == 13) || (reg == 14)) ? rUnd[reg - 13] = value : r[reg] = value;
		}
	}
}

u32 Arm7tdmi::getSPSRValue() {
	switch (CURRENT_MODE) {
	case ARM7TDMI_MODE_FIQ:return spsr[0];
	case ARM7TDMI_MODE_IRQ:return spsr[1];
	case ARM7TDMI_MODE_SVC:return spsr[2];
	case ARM7TDMI_MODE_ABT:return spsr[3];
	case ARM7TDMI_MODE_UND:return spsr[4];
	default:return cpsr;
	}
}

void Arm7tdmi::evaluateArm(u32 op) {
	if (!evalCondition(cpsr, op)) {
		return;
	}
	if (IS_BRANCH_AND_EXCHANGE(op)) {
		BX(op);
	}
	else if (IS_BLOCK_DATA_TRANSFER(op)) {
		if (IS_LOAD_INSTRUCTION(op)) {
			LDM(op);
		}
		else {
			STM(op);
		}
	}
	else if (IS_BRANCH(op)) {
		B_BL(op);
	}
	else if (IS_SOFTWARE_INTERRUPT(op)) {
		// Software Interrupt
	}
	else if (IS_SINGLE_DATA_TRANSFER(op)) {
		if (IS_LOAD_INSTRUCTION(op)) {
			LDR(op);
		}
		else {
			STR(op);
		}
	}
	else if (IS_MRS(op)) {
		//print op and opcode for now
		printf("MRS : op: %08x\n", op);
	}
	else if (IS_MSR_REG(op)) {
		// Move to SPSR from Immediate
		printf("MSR REG : op: %08x\n", op);
	}
	else if (IS_DATA_PROCESSING(op)) {
		if (IS_MSR_IMM(op)) {
			// Move to SPSR from Register
			printf("MSR IMM : op: %08x\n", op);
		}
		else
		{
			switch (ARM7TDMI_ARM_EXTRACT_OPCODE(op)) {
			case 0: AND(op); break;
			case 1: EOR(op); break;
			case 2: SUB(op); break;
			case 3: RSB(op); break;
			case 4: ADD(op); break;
			case 5: ADC(op); break;
			case 6: SBC(op); break;
			case 7: RSC(op); break;
			case 8: TST(op); break;
			case 9: TEQ(op); break;
			case 10: CMP(op); break;
			case 11: CMN(op); break;
			case 12: ORR(op); break;
			case 13: MOV(op); break;
			case 14: BIC(op); break;
			case 15: MVN(op); break;
			}
		}
	}
	else if (IS_HALFWORD_DATA_TRANSFER(op)) {
		// Halfword Data Transfer ( immediate OR register offset )
		if (IS_LOAD_INSTRUCTION(op)) {
			LDR2(op);
		}
		else
		{
			STR2(op);
		}
	}
	else if (IS_SINGLE_DATA_SWAP(op)) {
		if (IS_BYTE_TRANFER_INSTRUCTION(op)) {
			printf("SWPB : op: %08x\n", op);
			SWPB(op);
		}
		else {
			printf("SWP : op: %08x\n", op);
			SWP(op);
		}
	}
	else if (IS_MULTIPLY(op)) {
		execMultiply(op);
	}
	else {
		// Undefined
	}
}

Arm7tdmi::Arm7tdmi(Bus* bus) : bus(bus) {}

void Arm7tdmi::printRegsUserMode() {
	for (int i = 0; i < 16; i++) {
		printf("r%02d: %08x\n", i, r[i]);
	}
	printf("cpsr: %08x\n", cpsr);
}

int main() {
	Arm7tdmi cpu = Arm7tdmi(new Bus());
	cpu.cpsr |= T;

	while (0) {
		if (cpu.cpsr & T) { // Thumb mode
			u16 op = 0;// cpu.read16(r[15]);
			cpu.evaluateThumb(op);
			cpu.r[15] += 2;
		}
		else { // ARM mode
			u32 op = 0;// cpu.read32(r[15]);

			cpu.evaluateArm(op);

			cpu.r[15] += 4;
		}
	}

	//testWrite1(&cpu);

	//test1(&cpu);
   // cpu.printRegsUserMode();
	testMOVFlow(&cpu);
	//cpu.printRegsUserMode();

	testSWPBsimple(&cpu);
	cpu.printRegsUserMode();

}