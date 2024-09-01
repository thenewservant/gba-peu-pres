#include "bus/gba_bus.h"

u8 Bus::read8(u32 addr) {
#ifdef DEBUG
	printf("Reading 8 bits from %08x\n", addr);
#endif
	switch (addr & 0x0F000000) {
	case 0x00000000:
		return bios[addr];
	case 0x02000000:
		return *(ewram + (addr & 0x0003FFFF));
	case 0x03000000:
		return *(iwram + (addr & 0x00007FFF));
	case 0x04000000:
		return ioRead8(addr);
	case 0x05000000:
		return *(palette_ram + (addr & 0x000003FF));
	case 0x06000000:
		return *(vram + (addr & 0x0001FFFF)); /*TODO: check mirroring*/
	case 0x07000000:
		return *(oam + (addr & 0x000003FF));
	case 0x08000000:
	case 0x09000000:
		return *(rom + addr - 0x08000000); /*wait state 0*/
	case 0x0A000000:
	case 0x0B000000:
		return *(rom + addr - 0x0A000000); /*wait state 1*/
	case 0x0C000000:
	case 0x0D000000:
		return *(rom + addr - 0x0C000000); /*wait state 2*/
	case 0x0E000000:
	case 0x0F000000:
		return *(sram + (addr & 0x0000FFFF)); /*GamePak SRAM*/
	default:
		printf("Invalid memory access when reading 8 bits: %08xn", addr);
		return 0;
	}
}

u16 Bus::read16(u32 addr) {
	//printf("Reading 16 bits from %08x\n", addr);
#ifdef DEBUG
	printf("Reading 16 bits from %08x\n", addr);
#endif
	switch (addr & 0x0F000000) {
	case 0x00000000:
		return *(u16*)(bios + addr);
	case 0x02000000:
		return *(u16*)(ewram + (addr & 0x0003FFFF));
	case 0x03000000:
		return *(u16*)(iwram + (addr & 0x00007FFF));
	case 0x04000000:
		return ioRead16(addr);
	case 0x05000000:
		return *(u16*)(palette_ram + (addr & 0x000003FF));
	case 0x06000000:
		return *(u16*)(vram + vramMirroredAdress(addr)); /*TODO: check mirroring*/
	case 0x07000000:
		return *(u16*)(oam + (addr & 0x000003FF));
	case 0x08000000:
	case 0x09000000:
		return *(u16*)(rom + addr - 0x08000000); /*wait state 0*/
	case 0x0A000000:
	case 0x0B000000:
		return *(u16*)(rom + addr - 0x0A000000); /*wait state 1*/
	case 0x0C000000:
	case 0x0D000000:
		return *(u16*)(rom + addr - 0x0C000000); /*wait state 2*/
	default:
		printf("Invalid memory access when reading 16 bits: %08xn", addr);
		return 0;
	}
}

u32 Bus::read32(u32 addr) {
#ifdef DEBUG
	printf("Reading 32 bits from %08x\n", addr);
#endif
	addr = addr - (addr % 4);
	switch (addr & 0x0F000000) {
	case 0x00000000:
		if (addr & 0xFF000000) {
			printf("WARNING! Invalid u32 read attempted at %08X! returning dummy value\n", addr);
			return potHole;
		}
		return *(u32*)(bios + addr);
	case 0x02000000:
		return *(u32*)(ewram + (addr & 0x0003FFFF));
	case 0x03000000:
		return *(u32*)(iwram + (addr & 0x00007FFF));
	case 0x04000000:
		return ioRead32(addr);
	case 0x05000000:
		return *(u32*)(palette_ram + (addr & 0x000003FF));
	case 0x06000000:
		return *(u32*)(vram + vramMirroredAdress(addr)); /*TODO: check mirroring*/
	case 0x07000000:
		return *(u32*)(oam + (addr & 0x000003FF));
	case 0x08000000:
	case 0x09000000:
		return *(u32*)(rom + addr - 0x08000000); /*wait state 0*/
	case 0x0A000000:
	case 0x0B000000:
		return *(u32*)(rom + addr - 0x0A000000); /*wait state 1*/
	case 0x0C000000:
	case 0x0D000000:
		return *(u32*)(rom + addr - 0x0C000000); /*wait state 2*/
	default:
		printf("Invalid memory access when reading 32 bits: %08x\n", addr);
		return 0;
	}
}

u16 Bus::read16OAM(u32 addr) {
	return *(u16*)(oam + (addr & 0x000003FF));
}

u8 Bus::read8VRAM(u32 addr) {
	return *(vram + (addr & 0x0001FFFF));
}

u16 Bus::read16Palette(u32 addr) {
	return *(u16*)(palette_ram + (addr & 0x000003FF));
}