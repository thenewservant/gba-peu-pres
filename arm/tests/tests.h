#include "../arm7tdmi.h"
/*
add r0, r1, #123
umull r4, r5, r2, r0
smull r3, r1, r5, r4
umull r4, r5, r2, r1
umull r4, r5, r1, r0
add r3, r2, #12
mul r0, r4, r0

final state:
r2 = 3
r3 = f
others = 0
*/
void resetCPU(Arm7tdmi* cpu) {
    for (int i = 0; i < 16; i++) {
		cpu->r[i] = 0;
	}
    cpu->cpsr = 0;
}
void test1(Arm7tdmi* cpu) {
    cpu->r[0] = 1;
    cpu->r[1] = 2;
    cpu->r[2] = 3;
    cpu->r[3] = 4;

    //cpu->ADD(0xe281007b);
    cpu->evaluateArm(0xe0854092);
    cpu->evaluateArm(0xe0c13495);
    cpu->evaluateArm(0xe0854192);
    cpu->evaluateArm(0xe0854091);
    //cpu->ADD(0xe282300c);
    cpu->evaluateArm(0xe0000094);

}
void testWrite1(Arm7tdmi* cpu) {
    cpu->bus->write8(0x00000000, 0x12);
    cpu->bus->write8(0x00000001, 0x34);
    cpu->bus->write16(0x00000002, 0x5678);
    u16 read1 = cpu->bus->read16(0x00000000);
    u16 read2 = cpu->bus->read16(0x00000002);
    printf("%04x, %04x\n", read1, read2);
}

/*
r00:00000104
r01:00000100
r02:00000280
r03:00000a60
r04:00000f60
*/
void testMOV(Arm7tdmi* cpu) {
    cpu -> r[0] = 0;
    cpu->MOV(0xe3a00f41);
    cpu->MOV(0xe3a01c01);
    cpu->MOV(0xe3a02d0a);
    cpu->MOV(0xe3a03ea6);
    cpu->MOV(0xe3a04ef6);
}

void testMOVFlow(Arm7tdmi* cpu) {
    resetCPU(cpu);
    u32 instrs[] = {0xe3a00f41, 0xe3a01c01, 0xe3a02d0a, 0xe3a03ea6, 0xe3a04ef6, 0};
    for (int i = 0; i < 5; i++) {
		cpu->evaluateArm(instrs[i]);
    }
}

void testMRS_MSRDecode(Arm7tdmi* cpu) {
    resetCPU(cpu);
    u32 instrs[] = { 0xe10fa000, 0xe129f001 ,0xe329f017,0, 0 };
    for (int i = 0; i < 5; i++) {
        cpu->evaluateArm(instrs[i]);
    }
}

void testSWPBsimple(Arm7tdmi* cpu) {
    resetCPU(cpu);
    cpu->bus->write32(0x00000000, 0x12345678);
    cpu->bus->write32(0x00000004, 0xdeadbeef);
    cpu->SWPB(0xe1413093);
}

void testSWI(Arm7tdmi* cpu) {
	resetCPU(cpu);
	cpu->evaluateArm(0xef00dead);
}

void testRom1(Arm7tdmi* cpu) {
    resetCPU(cpu);
    cpu->wReg(0, 0x11111111);
    u32 instrs[] = {0xe3a00c01, 0xe3a01301, 0xe5810000, 0xe3a00f41, 0xe3a01301, 0xe5810008, 0xe3a00000, 0xe3a01405, 0xe3a02010, 0xe1c100b0};
    for (int i = 0; i < 10; i++) {
        cpu->evaluateArm(instrs[i]);
    }
}