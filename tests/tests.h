#ifndef TESTS_H
#define TESTS_H

#include "arm/arm7tdmi.h"




#include <cassert>

void test_SUB() {
    Arm7tdmi cpu = Arm7tdmi(nullptr);

    // Test SUB o� le d�bordement ne se produit pas
    cpu.wReg(1, 10);
    cpu.wReg(2, 5);
    cpu.SUB(0xE0411002); // SUB R1, R1, R2
    assert(cpu.rReg(1) == 5);
    assert((cpu.cpsr & V) == 0); // V flag should not be set

    // Test SUB o� le d�bordement se produit
    cpu.wReg(1, 0x80000000);
    cpu.wReg(2, 1);
    cpu.SUB(0xE0411002); // SUB R1, R1, R2
    assert(cpu.rReg(1) == 0x7FFFFFFF);
    assert((cpu.cpsr & V) == 0); // V flag should not be set
}

void test_CMP() {
	Arm7tdmi cpu = Arm7tdmi(nullptr);

	// Test CMP o� le d�bordement ne se produit pas
	cpu.wReg(1, 10);
	cpu.wReg(2, 5);
	cpu.CMP(0xE1510002); // CMP R1, R2
	assert(cpu.rReg(1) == 10);
	assert((cpu.cpsr & V) == 0); // V flag should not be set

	// Test CMP o� le d�bordement se produit
	cpu.wReg(1, 0x80000000);
	cpu.wReg(2, 1);
	cpu.evaluateArm(0xE1510002); // CMP R1, R2
	assert(cpu.rReg(1) == 0x80000000);
	assert((cpu.cpsr & V) == 0); // V flag should not be set

    cpu.wReg(1, 0x80000000);
	cpu.wReg(2, 0x80000000);
	cpu.evaluateArm(0xE1510002); // CMP R1, R2
	assert(cpu.rReg(1) == 0x80000000);
	assert((cpu.cpsr & N) == 0); // N flag should not  be set
    assert((cpu.cpsr & Z) == Z); //Z flag should be set
    assert((cpu.cpsr & C) == C); //C flag should be set

}

void test_SUB2() {
    Arm7tdmi cpu = Arm7tdmi(nullptr);

    // Test SUB avec un op�rande imm�diat
    cpu.wReg(1, 10);
    cpu.SUB(0xE2411005); // SUB R1, R1, #5
    assert(cpu.rReg(1) == 5);

    // Test SUB avec un op�rande de registre
    cpu.wReg(1, 10);
    cpu.wReg(2, 3);
    cpu.SUB(0xE0411002); // SUB R1, R1, R2
    assert(cpu.rReg(1) == 7);

    // Test SUB avec un d�calage logique � gauche
    cpu.wReg(1, 32);
    cpu.wReg(2, 1);
    cpu.SUB(0xE1A012A2); // SUB R1, R1, R2, LSL #5
    assert(cpu.rReg(1) == 0);

    // Test SUB avec un d�calage logique � droite
    cpu.wReg(1, 32);
    cpu.wReg(2, 64);
    cpu.SUB(0xe04110a2); // SUB R1, R1, R2, LSR #1
    assert(cpu.rReg(1) == 0);

    // Test SUB avec une rotation � droite
    cpu.wReg(1, 0x80000000);
    cpu.wReg(2, 0x40000000);
    cpu.SUB(0xe04110e2); // SUB R1, R1, R2, ROR #1
    assert(cpu.rReg(1) == 0x60000000);
}

void test_ADD() {
    Arm7tdmi cpu = Arm7tdmi(new Bus());

    // Test ADD avec un op�rande imm�diat
    cpu.wReg(1, 5);
    cpu.ADD(0xE2811007); // ADD R1, R1, #7
    assert(cpu.rReg(1) == 12);

    // Test ADD avec un op�rande de registre
    cpu.wReg(2, 3);
    cpu.ADD(0xE0801002); // ADD R1, R0, R2
    assert(cpu.rReg(1) == 3);

    // Test ADD avec un d�calage logique � gauche
    cpu.wReg(2, 1);
    cpu.ADD(0xe0801282); // ADD R1, R0, R2, LSL #5
    assert(cpu.rReg(1) == 32);

    // Test ADD avec un d�calage logique � droite
    cpu.wReg(2, 32);
    cpu.ADD(0xe08012a2); // ADD R1, R0, R2, LSR #5
    assert(cpu.rReg(1) == 1);

    // Test ADD avec une rotation � droite
    cpu.wReg(2, 0x80000000);
    cpu.ADD(0xe08010e2); // ADD R1, R0, R2, ROR #1
    assert(cpu.rReg(1) == 0x40000000);
}




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

void testAddVFLAG(Arm7tdmi* cpu) {
    resetCPU(cpu);
    cpu->evaluateArm(0xe3e00102);
    cpu->evaluateArm(0xe3a01001);
    cpu->evaluateArm(0xe0902001);
}

void test_LDM(Arm7tdmi* cpu) {
    // Initialiser le registre et la m�moire
    cpu->wReg(1, 0x1000); // Adresse de la m�moire
    cpu->bus->write32(0x1000, 0x12345678);
    cpu->bus->write32(0x1004, 0x9abcdef0);

    // Ex�cuter l'instruction LDM
    cpu->evaluateArm(0xE8910003); // LDM R1, {R0, R1}

    // V�rifier que les valeurs ont �t� correctement charg�es
    assert(cpu->rReg(0) == 0x12345678);
    assert(cpu->rReg(1) == 0x9abcdef0);
}

void test_LDM_IA_DB(Arm7tdmi* cpu) {
    // Initialiser le registre et la m�moire
    cpu->wReg(1, 0x1000); // Adresse de la m�moire
    cpu->bus->write32(0x1000, 0x12345678);
    cpu->bus->write32(0x1004, 0x9abcdef0);

    // Ex�cuter l'instruction LDM en mode IA
    cpu->evaluateArm(0xE8910003); // LDMIA R1!, {R0, R1}

    // V�rifier que les valeurs ont �t� correctement charg�es
    assert(cpu->rReg(0) == 0x12345678);
    assert(cpu->rReg(1) == 0x9abcdef0);

    // R�initialiser le registre et la m�moire
    cpu->wReg(0, 1);
    cpu->wReg(1, 0x0300001C); // Adresse de la m�moire
    cpu->bus->write32(0x03000014, 0xabcdef12);
    cpu->bus->write32(0x03000018, 0x3456789a);

    // Ex�cuter l'instruction LDM en mode DB
    cpu->evaluateArm(0xE9110003); // LDMDB R1!, {R0, R1}

    cpu->printRegsUserMode();
    // V�rifier que les valeurs ont �t� correctement charg�es
    assert(cpu->rReg(0) == 0xabcdef12);
    assert(cpu->rReg(1) == 0x3456789a);
}

void test_STRBT(Arm7tdmi* cpu) {


    // Initialiser le registre et la m�moire
    cpu->wReg(1, 0x12345678);
    cpu->wReg(2, 0x03000000); // Adresse de la m�moire
    cpu->bus->write8(0x03000000, 0x00);

    // Ex�cuter l'instruction STRBT
    cpu->evaluateArm(0xe4e21000); // STRBT R1, [R2]

    // V�rifier que la valeur a �t� correctement stock�e
    
    assert(cpu->bus->read8(0x03000000) == 0x78);
}

void test_STRBT_post_indexed(Arm7tdmi* cpu) {
    Bus* mem = cpu->bus;
    // Initialiser le registre et la m�moire
    cpu->wReg(1, 0x12345678);
    cpu->wReg(2, 0x1000); // Adresse de la m�moire
    cpu->wReg(3, 4); // Offset
    mem->write8(0x1000, 0x00);

    // Ex�cuter l'instruction STRBT avec adressage post-index�
    cpu->evaluateArm(0xe6e21003); // STRBT R1, [R2], R3

    // V�rifier que la valeur a �t� correctement stock�e
    assert(mem->read8(0x1004) == 0x78);

    // V�rifier que l'adresse de base a �t� mise � jour
    assert(cpu->rReg(2) == 0x1004);
}

void testSequence1(Arm7tdmi* cpu) {
    test_STRBT(cpu);
    printf("test STRBT: success\n");
    test_LDM(cpu);
    printf("test LDM: success\n");
    test_LDM_IA_DB(cpu);
    printf("test LDM IA DB: success\n");
    test_STRBT_post_indexed(cpu);
    printf("test STRBT post indexed: success\n");
}

void testWriteToEWRAM(Arm7tdmi* cpu) {
    cpu->r[0] = 0x02000000;
    cpu->r[1] = 0x12345678;
    cpu->evaluateArm(0xe5801000);
    printf("\n test read: %08X \n", cpu->bus->read32(0x02000000));
    assert (cpu->bus->read32(0x02000000) == 0x12345678);
}

void testInstr(Arm7tdmi* cpu) {
    cpu->evaluateArm(0xe5810000);
}



#endif // TESTS_H