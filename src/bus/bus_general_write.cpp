#include "bus/gba_bus.h"

void Bus::write8(u32 addr, u8 data) {
#ifdef DEBUG
	printf("Writing 8 bits from %08x\n", addr);
#endif
	switch (addr & 0x0F000000) {
	case 0x02000000:
		ewram[addr & 0x0003FFFF] = data;
		break;
	case 0x03000000:
		iwram[addr & 0x00007FFF] = data;
		break;
	case 0x04000000:
		*(writeIo(addr, data)) = data;
		break;
	case 0x0E000000:
	case 0x0F000000:
		sram[addr & 0x0000FFFF] = data; /*GamePak SRAM*/
		break;
	default:
		printf("Invalid memory access when writing 8 bits: %08x\n", addr);
		break;
	}
}

void Bus::write16(u32 addr, u16 data) {
#ifdef DEBUG
	printf("Writing %04x (16 bits) to %08x\n", data, addr);
#endif
	switch (addr & 0x0F000000) {
	case 0x02000000:
		*(u16*)(ewram + (addr & 0x0003FFFF)) = data;
		break;
	case 0x03000000:
		*(u16*)(iwram + (addr & 0x00007FFF)) = data;
		break;
	case 0x04000000:
		ioWrite16(addr, data);
		break;
	case 0x05000000:
		*(u16*)(palette_ram + (addr & 0x000003FF)) = data;
		break;
	case 0x06000000:
		*(u16*)(vram + vramMirroredAdress(addr)) = data;
		break;
	case 0x07000000:
		*(u16*)(oam + (addr & 0x000003FF)) = data;
		break;
	default:
		printf("Invalid memory access when writing 16 bits: %08x\n", addr);
		break;
	}
}

void Bus::write32(u32 addr, u32 data) {
#ifdef DEBUG
	printf("Writing %08x (32 bits) to %08x\n", data, addr);
#endif
	u32 addr2 = addr - (addr % 4);
	switch (addr & 0x0F000000) {
	case 0x02000000:
		*(u32*)(ewram + (addr2 & 0x0003FFFF)) = data;
		break;
	case 0x03000000:
		*(u32*)(iwram + (addr2 & 0x00007FFF)) = data;
		break;
	case 0x04000000:
		ioWrite32(addr, data);
		break;
	case 0x05000000:
		*(u32*)(palette_ram + (addr2 & 0x000003FF)) = data;
		break;
	case 0x06000000:
		*(u32*)(vram + vramMirroredAdress(addr2)) = data;
		break;
	case 0x07000000:
		*(u32*)(oam + (addr2 & 0x000003FF)) = data;
		break;
	default:
		printf("Invalid memory access when writing 32 bits: %08x\n", addr);
		break;
	}
}
