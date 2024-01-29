#include "gba_bus.h"

 constexpr u8* Bus::getMemoryChunkFromAddress(u32 add) {
	switch (add & 0x0F000000) {
	case 0x00000000:
		return bios;
	case 0x02000000:
		return ewram;
	case 0x03000000:
		return iwram;
	case 0x05000000:
		return palette_ram;
	case 0x06000000:
		return vram;
	case 0x07000000:
		return oam;
	default:
		return nullptr;
	}
}

u8 Bus::read8(u32 addr) {
	switch (addr & 0x0F000000) {
	case 0x00000000:
		return bios[addr];
	case 0x02000000:
		return ewram[addr];
	case 0x03000000:
		return iwram[addr];
	case 0x05000000:
		return palette_ram[addr];
	case 0x06000000:
		return vram[addr];
	case 0x07000000:
		return oam[addr];
	default:
		return 0;
	}
}

u16 Bus::read16(u32 addr) {
	switch (addr & 0x0F000000) {
	case 0x00000000:
		return *(u16*)(bios + addr);
	case 0x02000000:
		return *(u16*)(ewram + addr);
	case 0x03000000:
		return *(u16*)(iwram + addr);
	case 0x05000000:
		return *(u16*)(palette_ram + addr);
	case 0x06000000:
		return *(u16*)(vram + addr);
	case 0x07000000:
		return *(u16*)(oam + addr);
	default:
		return 0;
	}
}

u32 Bus::read32(u32 addr) {
	switch (addr & 0x0F000000) {
	case 0x00000000:
		return *(u32*)(bios + addr);
	case 0x02000000:
		return *(u32*)(ewram + addr);
	case 0x03000000:
		return *(u32*)(iwram + addr);
	case 0x05000000:
		return *(u32*)(palette_ram + addr);
	case 0x06000000:
		return *(u32*)(vram + addr);
	case 0x07000000:
		return *(u32*)(oam + addr);
	default:
		return 0;
	}
}

void Bus::write8(u32 addr, u8 data) {
	switch (addr & 0x0F000000) {
	case 0x00000000:
		bios[addr] = data;
		break;
	case 0x02000000:
		ewram[addr] = data;
		break;
	case 0x03000000:
		iwram[addr] = data;
		break;
	case 0x05000000:
		palette_ram[addr] = data;
		break;
	case 0x06000000:
		vram[addr] = data;
		break;
	case 0x07000000:
		oam[addr] = data;
		break;
	default:
		break;
	}
}

void Bus::write16(u32 addr, u16 data) {
	*(u16*)(getMemoryChunkFromAddress(addr) + addr) = data;
}

void Bus::write32(u32 addr, u32 data) {
	switch (addr & 0x0F000000) {
	case 0x00000000:
		*(u32*)(bios + addr) = data;
		break;
	case 0x02000000:
		*(u32*)(ewram + addr) = data;
		break;
	case 0x03000000:
		*(u32*)(iwram + addr) = data;
		break;
	case 0x05000000:
		*(u32*)(palette_ram + addr) = data;
		break;
	case 0x06000000:
		*(u32*)(vram + addr) = data;
		break;
	case 0x07000000:
		*(u32*)(oam + addr) = data;
		break;
	default:
		break;
	}
}