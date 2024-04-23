#include "arm7tdmi.h"
#include <windows.h>
#include "../ppu/ppu.h"

/* arm7tdmi.cpp
* General operation, instruction decoding, register access, etc.
*/

#define IS_BRANCH_AND_EXCHANGE(op)    ((op & 0x0FFFFFF0) == 0x012FFF10)
#define IS_SOFTWARE_INTERRUPT(op)     ((op & 0x0F000000) == 0x0F000000)
#define IS_BRANCH(op)                 ((op & 0x0E000000) == 0x0A000000)
#define IS_BLOCK_DATA_TRANSFER(op)    ((op & 0x0E000000) == 0x08000000)
#define IS_SINGLE_DATA_TRANSFER(op)   ((op & 0x0C000000) == 0x04000000)
#define IS_UNDEFINED(op)              ((op & 0x0E000010) == 0x06000010)
#define IS_DATA_PROCESSING(op)        ((op & 0x0C000000) == 0x00000000)
#define IS_HALFWORD_DAT_TRANS_REG(op) ((op & 0x0E400F90) == 0x00000090)
#define IS_HALFWORD_DAT_TRANS_IMM(op) ((op & 0x0E400090) == 0x00400090)
#define IS_SINGLE_DATA_SWAP(op)       ((op & 0x0FB00FF0) == 0x01000090)
#define IS_MULTIPLY(op)               ((op & 0x0F0000F0) == 0x00000090)
#define IS_MRS(op)                    ((op & 0x0FBF0FFF) == 0x010F0000)
#define IS_MSR_IMM(op)                ((op & 0x0FB0F000) == 0x0320F000)
#define IS_MSR_REG(op)                ((op & 0x0FB00FF0) == 0x01200000)

#define IS_LOAD_INSTRUCTION(op)				      (op & BIT(20))
#define IS_BYTE_TRANFER_INSTRUCTION(op)			  (op & BIT(22))

#define FLAG_SET(x) ((cpsr & x) > 0)
#define FLAG_UNSET(x) ((cpsr & x) == 0)

bool evalCondition(u32 cpsr, u32 op) {
	switch (op & 0xF0000000) {
	case ARM7TDMI_CONDITION_EQ:	return FLAG_SET(Z);
	case ARM7TDMI_CONDITION_NE:	return FLAG_UNSET(Z);
	case ARM7TDMI_CONDITION_CS:	return FLAG_SET(C);
	case ARM7TDMI_CONDITION_CC:	return FLAG_UNSET(C);
	case ARM7TDMI_CONDITION_MI:	return FLAG_SET(N);
	case ARM7TDMI_CONDITION_PL:	return FLAG_UNSET(N);
	case ARM7TDMI_CONDITION_VS:	return FLAG_SET(V);
	case ARM7TDMI_CONDITION_VC:	return FLAG_UNSET(V);
	case ARM7TDMI_CONDITION_HI:	return FLAG_SET(C) && FLAG_UNSET(Z);
	case ARM7TDMI_CONDITION_LS:	return FLAG_UNSET(C) || FLAG_SET(Z);
	case ARM7TDMI_CONDITION_GE:	return (FLAG_SET(N) && FLAG_SET(V)) || (FLAG(N) == FLAG(V));
	case ARM7TDMI_CONDITION_LT:	return (FLAG_UNSET(N) && FLAG_SET(V)) || (FLAG_SET(N) && FLAG_UNSET(V));
	case ARM7TDMI_CONDITION_GT:	return FLAG_UNSET(Z) && ((FLAG_SET(N) && FLAG_SET(V)) || (FLAG(N) == FLAG(V)));
	case ARM7TDMI_CONDITION_LE:	return FLAG_SET(Z) || !((FLAG_SET(N) && FLAG_SET(V)) || (FLAG(N) == FLAG(V)));
	case ARM7TDMI_CONDITION_AL:	return true;
	case ARM7TDMI_CONDITION_NV:
	default:
		return false;
	}
}

bool Arm7tdmi::currentModeHasSPSR() {
	switch (CURRENT_MODE) {
	case ARM7TDMI_MODE_FIQ:
	case ARM7TDMI_MODE_IRQ:
	case ARM7TDMI_MODE_SVC:
	case ARM7TDMI_MODE_ABT:
	case ARM7TDMI_MODE_UND:
		return true;
	default:
		return false;
	}
}

u32 Arm7tdmi::rReg(u8 reg) {
	if (reg < 8) {
		return r[reg];
	}
	if (reg == 15) {
		return r[15] + 8;
	}
	switch (CURRENT_MODE) {
	case ARM7TDMI_MODE_USER:
	case ARM7TDMI_MODE_SYS:
		return r[reg];
	case ARM7TDMI_MODE_FIQ:
		return rFiq[reg - 8];
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

u32 Arm7tdmi::rRegThumb(u8 reg) {
	if (reg == 15) {
		return r[15] + 4;
	}
	else {
		return rReg(reg);
	}
}

u32 Arm7tdmi::rRegMode(u8 reg, u8 mode) {
	if (reg == 15) {
		return r[15] + 8;
	}
	switch (mode) {
	case ARM7TDMI_MODE_USER:
	case ARM7TDMI_MODE_SYS:
		return r[reg];
	case ARM7TDMI_MODE_FIQ:
		return rFiq[reg - 8];
	case ARM7TDMI_MODE_SVC:
		return ((reg >= 13)) ? rSvc[reg - 13] : r[reg];
	case ARM7TDMI_MODE_ABT:
		return ((reg >= 13)) ? rAbt[reg - 13] : r[reg];
	case ARM7TDMI_MODE_UND:
		return ((reg >= 13)) ? rUnd[reg - 13] : r[reg];
	default: return 0;
	}
}

void Arm7tdmi::wRegMode(u8 reg, u32 data, u8 mode) {
	if (reg == 15) {
		r[15] = data;
	}
	else {
		switch (mode) {
		case ARM7TDMI_MODE_USER:
		case ARM7TDMI_MODE_SYS:
			r[reg] = data;
			break;
		case ARM7TDMI_MODE_FIQ:
			rFiq[reg - 8] = data;
			break;
		case ARM7TDMI_MODE_SVC:
			((reg >= 13)) ? rSvc[reg - 13] = data : r[reg] = data;
			break;
		case ARM7TDMI_MODE_ABT:
			((reg >= 13)) ? rAbt[reg - 13] = data : r[reg] = data;
			break;
		case ARM7TDMI_MODE_UND:
			((reg >= 13)) ? rUnd[reg - 13] = data : r[reg] = data;
			break;
		default: break;
		}
	}
}


void Arm7tdmi::wReg(u8 reg, u32 value) {
	if (reg < 8) {
		r[reg] = value;
	}

	else {
		if (reg == 15) {
			r[15] = (value & 0xFFFFFFFE);
			pcHasChanged = true;
		} else{

			switch (CURRENT_MODE) {

			case ARM7TDMI_MODE_USER:
			case ARM7TDMI_MODE_SYS:
				r[reg] = value;
				break;
			case ARM7TDMI_MODE_FIQ:
				 rFiq[reg - 8] = value;
				break;
			case ARM7TDMI_MODE_SVC:
				((reg == 13) || (reg == 14)) ? rSvc[reg - 13] = value : r[reg] = value;
				break;
			case ARM7TDMI_MODE_ABT:
				((reg == 13) || (reg == 14)) ? rAbt[reg - 13] = value : r[reg] = value;
				break;
			case ARM7TDMI_MODE_IRQ:
				((reg == 13) || (reg == 14)) ? rIrq[reg - 13] = value : r[reg] = value;
				break;
			case ARM7TDMI_MODE_UND:
				((reg == 13) || (reg == 14)) ? rUnd[reg - 13] = value : r[reg] = value;
				break;
			}
		}
	}
}

void Arm7tdmi::SWI(u32 op) {
	printf("ARM SWI : op: %08x\n", op);
	rSvc[1] = r[15] + 4;
	spsr[2] = cpsr;
	cpsr &= ~(ARM7TDMI_MODE_MASK | BIT(5)  | BIT(9));
	cpsr |= BIT(7);
	cpsr |= ARM7TDMI_MODE_SVC;
	wReg(15, 0x8);
}

u32 Arm7tdmi::getSPSRValue() {
	switch (CURRENT_MODE) {
	case ARM7TDMI_MODE_FIQ:return spsr[0];
	case ARM7TDMI_MODE_IRQ:return spsr[1];
	case ARM7TDMI_MODE_SVC:return spsr[2];
	case ARM7TDMI_MODE_ABT:return spsr[3];
	case ARM7TDMI_MODE_UND:return spsr[4];
	}
	return cpsr;
}

void Arm7tdmi::setSPSRValue(u32 value) {
	switch (CURRENT_MODE) {
	case ARM7TDMI_MODE_FIQ:spsr[0] = value; break;
	case ARM7TDMI_MODE_IRQ:spsr[1] = value;	break;
	case ARM7TDMI_MODE_SVC:spsr[2] = value; break;
	case ARM7TDMI_MODE_ABT:spsr[3] = value; break;
	case ARM7TDMI_MODE_UND:spsr[4] = value; break;
	default: break;
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
		SWI(op);
	}
	else if (IS_UNDEFINED(op)) {
		printf("OP is of the UNDEFINED format: %08x\n", op);
		exit(2);
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
		MRS(op);
	}
	else if (IS_MSR_REG(op)) {
		MSR_REG(op);
	}
	else if (IS_MULTIPLY(op)) {
		execMultiply(op);
	}
	else if (IS_HALFWORD_DAT_TRANS_REG(op)) {
		if (IS_LOAD_INSTRUCTION(op)) {
			LDR2(op);
		}
		else
		{
			STR2(op);
		}
	}
	else if (IS_HALFWORD_DAT_TRANS_IMM(op)) {		// Halfword Data Transfer ( immediate OR register offset )
		if (IS_LOAD_INSTRUCTION(op)) { LDR2(op); }
		else { STR2(op); }
	}
	else if (IS_SINGLE_DATA_SWAP(op)) {
		if (IS_BYTE_TRANFER_INSTRUCTION(op)) { SWPB(op); }
		else { SWP(op); }
	}
	else if (IS_DATA_PROCESSING(op)) {

		if (IS_MSR_IMM(op)) {
			MSR_IMM(op);
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
	else {
		printf("UNRECOGNIZED op: %08x\n", op);
		exit(1);
	}
}

void Arm7tdmi::setPPU(Ppu* ppu) {
	this->ppu = ppu;
}

Arm7tdmi::Arm7tdmi(Bus* bus) : bus(bus), cpsr(0), spsr{ 0 }, r{ 0 }, rFiq{ 0 }, rSvc{ 0 }, rAbt{ 0 }, rIrq{ 0 }, pcHasChanged{ false}
{
	r[13] = BOOT_SP_USR;
	rSvc[0] = BOOT_SP_SVC;
	rIrq[0] = BOOT_SP_IRQ;
	this->ppu = ppu;
	r[15] = 0x08000000;
	cpsr = 0x0000005f;
}

void Arm7tdmi::tick() {
	static u32 step = 0;
	if (this->cpsr & T) { // Thumb mode
		u16 op = bus->read16(r[15] - 4);
#ifdef DEBUG
		printf("PC: %08x\n", r[15]);
		printf("op: %04x\n", op);
#endif
		this->evaluateThumb(op);
		this->r[15] += 2;
	}
	else { // ARM mode
		u32 op = bus->read32(r[15]);
		while (op == 0) {
			printf("FATAL: OPCODE 0 @ r15 = %08x\n", r[15]);

		}
#ifdef DEBUG
		printf("PC: %08x\n", r[15]);
		printf("op: %08x\n", op);
#endif

		this->evaluateArm(op);
		if (!pcHasChanged) {
			this->r[15] += 4;
		}
#ifdef DEBUG
		printf("pcHascChanged? %s\n", pcHasChanged ? "true" : "false");
#endif
		
		pcHasChanged = false;
	}
	step++;
	static u64 nbShots = 0;
	if ((step % 4) == 0) {
		//printf("nbShots: %lld\n", nbShots++);
		ppu->tick();
		step = 0;
	}
}

void Arm7tdmi::printRegsUserMode() {
	printf("\n");
	for (int i = 0; i < 15; i++) {
		if (rReg(i))printf("r%02d: %08x\n", i, rReg(i));
	}
	printf("fake PC: %08x\nreal PC: %08x\n", rReg(15), r[15]);
	printf("\n");
	printf("cpsr: %08x\n", cpsr);
}