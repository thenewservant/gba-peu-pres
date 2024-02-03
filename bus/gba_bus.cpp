#include "gba_bus.h"
#include "../ppu/ppu.h"
#pragma warning(disable:4996)

u8* Bus::ioAccess(u32 add) {
	if (add <= 0x04000056) {
		return ppu->readIO(add);
	}
	else if (0x040000A8) {

	}
	exit(1);
	return nullptr;
}

 constexpr u8* Bus::getMemoryChunkFromAddress(u32 add) {
	switch (add & 0x0F000000) {
	case 0x00000000:
		return bios + add;
	case 0x02000000:
		return ewram + add - 0x02000000;
	case 0x03000000:
		return iwram + add - 0x03000000;
	case 0x04000000:
		return ioAccess(add);
	case 0x05000000:
		return palette_ram + add - 0x05000000;
	case 0x06000000:
		return vram + add - 0x06000000;
	case 0x07000000:
		return oam + add - 0x07000000;
	case 0x08000000:
		return rom + add - 0x08000000;
	default:
		return nullptr;
	}
}

u8 Bus::read8(u32 addr) {
	return *(getMemoryChunkFromAddress(addr));
}

u16 Bus::read16(u32 addr) {
	return *(u16*)(getMemoryChunkFromAddress(addr));
}

u32 Bus::read32(u32 addr) {
	return *(u32*)(getMemoryChunkFromAddress(addr));
}

void Bus::write8(u32 addr, u8 data) {
	*(getMemoryChunkFromAddress(addr)) = data;
}

void Bus::write16(u32 addr, u16 data) {
#ifdef DEBUG
	printf("Writing %04x (16 bits) to %08x\n", data, addr);
#endif
	*(u16*)(getMemoryChunkFromAddress(addr)) = data;
}

void Bus::write32(u32 addr, u32 data) {
#ifdef DEBUG
	printf("Writing %08x to %08x\n", data, addr);
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