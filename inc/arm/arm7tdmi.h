#ifndef ARM7TDMI_H
#define ARM7TDMI_H

#include "common/types.h"
#include "bus/gba_bus.h"

enum ARM7TDMI_MODE {
	ARM7TDMI_MODE_USER = 0x10,
	ARM7TDMI_MODE_FIQ = 0x11,
	ARM7TDMI_MODE_IRQ = 0x12,
	ARM7TDMI_MODE_SVC = 0x13,
	ARM7TDMI_MODE_ABT = 0x17,
	ARM7TDMI_MODE_UND = 0x1B,
	ARM7TDMI_MODE_SYS = 0x1F
};

#define ARM7TDMI_MODE_MASK 0x1F
#define CURRENT_MODE (cpsr & 0x1F)
#define CURRENT_MODE_HAS_SPSR ((CURRENT_MODE != ARM7TDMI_MODE_USER) && (CURRENT_MODE != ARM7TDMI_MODE_SYS))

#define MASK_32BIT 0xFFFFFFFF
#define MASK_16BIT 0xFFFF

#define BIT(x) (1 << (x))
#define FLAG(x) (cpsr & (x))

#define ARM7TDMI_ARM_EXTRACT_OPCODE(op) ((op >> 21) & 0xF)

#define BOOT_SP_SVC 0x03007FE0
#define BOOT_SP_IRQ 0x03007FA0
#define BOOT_SP_USR 0x03007F00

#define SWI_HANDLER_VECTOR 0x08

#define FLAG_SET(x) ((cpsr & x) > 0)
#define FLAG_UNSET(x) ((cpsr & x) == 0)

enum ARM7TDMI_CONDITION {
	ARM7TDMI_CONDITION_EQ = 0x0, // Z set
	ARM7TDMI_CONDITION_NE = 0x1, // Z clear
	ARM7TDMI_CONDITION_CS = 0x2, // C set
	ARM7TDMI_CONDITION_CC = 0x3, // C clear
	ARM7TDMI_CONDITION_MI = 0x4, // N set
	ARM7TDMI_CONDITION_PL = 0x5, // N clear
	ARM7TDMI_CONDITION_VS = 0x6, // V set
	ARM7TDMI_CONDITION_VC = 0x7, // V clear
	ARM7TDMI_CONDITION_HI = 0x8, // C set and Z clear
	ARM7TDMI_CONDITION_LS = 0x9, // C clear or Z set
	ARM7TDMI_CONDITION_GE = 0xA, // N equals V
	ARM7TDMI_CONDITION_LT = 0xB, // N not equal to V
	ARM7TDMI_CONDITION_GT = 0xC, // Z clear, and either N equals V, or N set and V clear
	ARM7TDMI_CONDITION_LE = 0xD, // Z set, or N not equal to V
	ARM7TDMI_CONDITION_AL = 0xE, // Always (unconditional)
	ARM7TDMI_CONDITION_NV = 0xF  // never (ARMv1,v2 only) (Reserved ARMv3 and up)
};

enum CPSR_FLAGS {
	N = 0x80000000,
	Z = 0x40000000,
	C = 0x20000000,
	V = 0x10000000,
	I = 0x00000080, // Disable IRQ
	F = 0x00000040, // Disable (Fast IRQ)
	T = 0x00000020, // Thumb mode (0 = ARM, 1 = THUMB)
	M = 0x0000001F  // Mode bits (see ARM7TDMI_MODE)
};

class Ppu;

class Arm7tdmi {
public:
	Bus* bus;
	Ppu* ppu;
	u32 r[16]; // Standard System/User mode r0 - r15 registers
	u32 rFiq[7]; // FIQ mode r8 - r14 registers
	u32 rIrq[2]; // IRQ mode r13 - r14 registers
	u32 rSvc[2]; // Supervisor mode r13 - r14 registers
	u32 rAbt[2]; // Abort mode r13 - r14 registers 
	u32 rUnd[2]; // Undefined mode r13 - r14 registers
	u32 cpsr; // Current Program Status Register
	u32 spsr[5]; // FIQ, IRQ, SVC, ABT, UND

	bool pcHasChanged;
	Arm7tdmi(Bus* bus);
	void setPPU(Ppu* ppu);
	void tick();

	u32 getPCInArmOrThumb();

	// reads register reg of the current mode
	u32 rReg(u8 reg);
	u32 rRegThumb(u8 reg);

	// reads register reg of the specified mode
	u32 rRegMode(u8 reg, u8 mode);
	void wRegThumb(u8 reg, u32 data);
	void wRegMode(u8 reg, u32 data, u8 mode);

	// writes value to register reg of the current mode
	void wReg(u8 reg, u32 value);
	void SWI(u32 op);

	// returns the value of the SPSR register of the current mode.
	// (CPSR if no SPSR in current mode)
	u32 getSPSRValue() const;
	void setSPSRValue(u32 value);
	bool currentModeHasSPSR();

	void B_BL(u32 op); // merged B and BL altogether
	void BX(u32 op);

	void printRegsUserMode();
	void execMultiply(u32 op);
	void MULT_SHORT(u32 op);
	void SMULL(u32 op);
	void UMULL(u32 op);
	void UMLAL(u32 op);

	void SMLAL(u32 op);
	void MULT_LONG(u32 op);
	void evaluateArm(u32 op);

	void evaluateThumb(u16 op);

	// Data Processing
	void AND(u32 op);
	void EOR(u32 op);
	void SUB(u32 op);
	void RSB(u32 op);
	void ADD(u32 op);
	void ADC(u32 op);
	void SBC(u32 op);
	void RSC(u32 op);
	void TST(u32 op);
	void TEQ(u32 op);
	void CMP(u32 op);
	void CMN(u32 op);
	void ORR(u32 op);
	void MOV(u32 op);
	void BIC(u32 op);
	void MVN(u32 op);

	void MSR(u32 operand, u32 op);

	void LDM(u32 op);
	u32 getNewBase(u32 op);
	void STM(u32 op);

	bool getAddressMode2(u32& op, u32& adress, const u32& rnVal, u32* thingToWrite);

	void LDR(u32 op);
	void STR(u32 op);

	void STR2(u32 op);
	void LDR2(u32 op);

	void SWP(u32 op);
	void SWPB(u32 op);

	void MSR_IMM(u32 op);
	void MSR_REG(u32 op);

	void MRS(u32 op);

	u32 aluReadRn(u32 op);

	void checkCPSR_DP(u32& op, const u8 shifterCarryOut);
	void TB_COND_BRANCH(u16 op);
	void TB_UNCOND_BRANCH(u16 op);
	void TB_BL(u16 op);
	void TB_HIGH_REG_OPERATION(u16 op);
	void TB_LDRPC(u16 op);
	void TB_LDR_STR_RELATIVE(u16 op);
	void TB_LDR_STR_SE_HW(u16 op);
	void TB_LDR_STR_IMMEDIATE(u16 op);
	void TB_LDRH_STRH(u16 op);
	void TB_LDRSP_STRSP(u16 op);
	void TB_LDMIA_STMIA(u16 op);
	void TB_GET_REL_ADDR(u16 op);
	void TB_ADD_OFFSET_SP(u16 op);
	void TB_PUSH(u16 op);
	void TB_POP(u16 op);
	void TB_MOVE_SHIFTED_REG(u16 op);
	void TB_ALU_OP(u16 op);
	void TB_IMMEDIATE_OPERATION(u16 op);
	void TB_ADD_SUBSTRACT(u16 op);
};

bool evalCondition(u32 cpsr, u32 op);

#endif