#include "gba_bus.h"
#include "../ppu/ppu.h"
#pragma warning(disable:4996)

inline u8* Bus::ioAccess(u32 add) {
	if (add <= 0x04000056) {
		return ppu->readIO(add);
	}
	else if (add >=0x4000200) {
		u16 addr = add & 0xF00;
		if (addr == 0x200) {
			return intCtrl.array + (add & 0x000000FF);
		}
		else if (addr == 0x300){
			return pwrStatus.array + (add & 0x000000FF);
		}
		else if (addr == 0x400) { // supposedly a bug, unused and undocumented access
			return (u8*)&potHole;
		}
		else if ((addr & 0x0F00FFFF) == 0x04000800) {
			return (u8*)&internalMemoryControl;
		}
		
	}
	printf("IO Access not implemented yet: %08x\n", add);
	exit(1);
	return nullptr;
}

inline u8* Bus::getMemoryChunkFromAddress(u32 add) {
	switch (add & 0x0F000000) {
	case 0x00000000:
		return bios + add;
	case 0x02000000:
		return ewram + (add & 0x0003FFFF);
	case 0x03000000:
		return iwram + (add & 0x00007FFF);
	case 0x04000000:
		return ioAccess(add);
	case 0x05000000:
		return palette_ram + (add & 0x000003FF);
	case 0x06000000:
		return vram + (add & 0x00017FFF);
	case 0x07000000:
		return oam + (add & 0x000003FF);
	case 0x08000000:
	case 0x09000000:
		return rom + add - 0x08000000;// wait state 0
	case 0x0A000000:
	case 0x0B000000:
		return rom + add - 0x0A000000; // wait state 1
	case 0x0C000000:
	case 0x0D000000:
		return rom + add - 0x0C000000; // wait state 2
	case 0x0E000000:
	case 0x0F000000:
		return sram + (add & 0x0000FFFF); // GamePak SRAM
	default:
		return nullptr;
	}
}

inline u8* Bus::get8bitWritableChunk(u32 add) {
	switch (add & 0x0F000000) {
	case 0x02000000:
		return ewram + (add & 0x0003FFFF);
	case 0x03000000:
		return iwram + (add & 0x00007FFF);
	case 0x04000000:
		return ioAccess(add);
	case 0x0E000000:
	case 0x0F000000:
		return sram + (add & 0x0000FFFF); // GamePak SRAM
	default:
		return (u8*)&potHole;
	}
}

u8 Bus::read8(u32 addr) {
#ifdef DEBUG
	printf("Reading 8 bits from %08x\n", addr);
#endif
	return *(getMemoryChunkFromAddress(addr));
}

u16 Bus::read16(u32 addr) {
	//printf("Reading 16 bits from %08x\n", addr);
#ifdef DEBUG
	printf("Reading 16 bits from %08x\n", addr);
#endif
	return *(u16*)(getMemoryChunkFromAddress(addr));
}

u32 Bus::read32(u32 addr) {
#ifdef DEBUG
	printf("Reading 32 bits from %08x\n", addr);
#endif
	return *(u32*)(getMemoryChunkFromAddress(addr));
}

void Bus::write8(u32 addr, u8 data) {
#ifdef DEBUG
	printf("Writing 8 bits from %08x\n", addr);
#endif
	*(get8bitWritableChunk(addr)) = data;
}

void Bus::write16(u32 addr, u16 data) {
#ifdef DEBUG
	printf("Writing %04x (16 bits) to %08x\n", data, addr);
#endif
	*(u16*)(getMemoryChunkFromAddress(addr)) = data;
}

void Bus::write32(u32 addr, u32 data) {
#ifdef DEBUG
	printf("Writing %08x (32 bits) to %08x\n", data, addr);
#endif
	*(u32*)(getMemoryChunkFromAddress(addr)) = data;
}

void Bus::loadGamePack(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	if (fp == nullptr) {
		printf("Failed to open file %s\n", filename);
		return;
	}

	u32 cursor = 0;
	while (!feof(fp)) {
		fread(rom+cursor++, sizeof(u8), 1, fp);
	}
	romSizeInBytes = cursor;
}

void Bus::loadBios(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	if (fp == nullptr) {
		printf("Failed to open file %s\n", filename);
		return;
	}

	u32 cursor = 0;
	while (!feof(fp)) {
		fread(bios + cursor++, sizeof(u8), 1, fp);
	}
}