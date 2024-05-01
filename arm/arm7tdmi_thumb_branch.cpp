#include "arm7tdmi.h"

#define GET_BRANCH_OPCODE(op) ((op >> 8) & 0xF)
#define TEST_BRANCH(cond) if(cond){wReg(15, (rRegThumb(15) + (s32)(sOffSet << 1)));}


void Arm7tdmi::TB_COND_BRANCH(u16 op) {
    s32 sOffSet = (s32)(s8)(op & 0xff);
    switch (GET_BRANCH_OPCODE(op)) {
    case ARM7TDMI_CONDITION_EQ:	TEST_BRANCH(FLAG_SET(Z)); break;
    case ARM7TDMI_CONDITION_NE:	TEST_BRANCH(FLAG_UNSET(Z)); break;
    case ARM7TDMI_CONDITION_CS:	TEST_BRANCH(FLAG_SET(C)); break;
    case ARM7TDMI_CONDITION_CC:	TEST_BRANCH(FLAG_UNSET(C)); break;
    case ARM7TDMI_CONDITION_MI:	TEST_BRANCH(FLAG_SET(N)); break;
    case ARM7TDMI_CONDITION_PL:	TEST_BRANCH(FLAG_UNSET(N)); break;
    case ARM7TDMI_CONDITION_VS:	TEST_BRANCH(FLAG_SET(V)); break;
    case ARM7TDMI_CONDITION_VC:	TEST_BRANCH(FLAG_UNSET(V)); break;
    case ARM7TDMI_CONDITION_HI:	TEST_BRANCH(FLAG_SET(C) && FLAG_UNSET(Z)); break;
    case ARM7TDMI_CONDITION_LS:	TEST_BRANCH(FLAG_UNSET(C) || FLAG_SET(Z)); break;
    case ARM7TDMI_CONDITION_GE:	TEST_BRANCH(((FLAG_SET(N) && FLAG_SET(V)) || (FLAG_UNSET(N) && FLAG_UNSET(V)))); break;
    case ARM7TDMI_CONDITION_LT:	TEST_BRANCH((FLAG_UNSET(N) && FLAG_SET(V)) || (FLAG_SET(N) && FLAG_UNSET(V))); break;
    case ARM7TDMI_CONDITION_GT:	TEST_BRANCH(FLAG_UNSET(Z) && ((FLAG_SET(N) && FLAG_SET(V)) || (FLAG_UNSET(N) && FLAG_UNSET(V)))); break;
    case ARM7TDMI_CONDITION_LE:	TEST_BRANCH(FLAG_SET(Z) || !((FLAG_SET(N) && FLAG_SET(V)) || (FLAG(N) == FLAG(V)))); break;
    default:
        break;
    }
}

void Arm7tdmi::TB_UNCOND_BRANCH(u16 op) {
    s32 sOffSet = ((s32)(s16)((op & 0x7FF) << 5))>>5;
    TEST_BRANCH(true);
}

void Arm7tdmi::TB_BL(u16 op) {
    u16 offset11 = op & 0x7ff;
    if (op & BIT(11)) {
        printf("BL11\n");
        u32 oldPc = r[15] + 2;
        wRegThumb(15, rRegThumb(14) + (offset11 << 1));
        r[14] = (oldPc + 2) | 1;
    }
    else {
        printf("BL10\n");
        s16 signExtendedOffset =( offset11 & BIT(10)) ? offset11 | 0xF800 : offset11;
        r[14] = rRegThumb(15) + (signExtendedOffset<<12);
    }
}