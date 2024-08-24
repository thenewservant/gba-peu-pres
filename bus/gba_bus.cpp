#include "gba_bus.h"
#include "../ppu/ppu.h"

#pragma warning(disable:4996)

#define DMA_FIRST_MAP_ADDRESS 0x040000B0
#define DMA_LAST_MAP_ADDRESS 0x040000DE


Bus::Bus() {
	this->timerManager = new TimerManager();
	for (int i = 0; i < 4; i++) {
		enum DMA_NB dmaId = (enum DMA_NB)i;
		dmaArray[i] = Dma(dmaId);
		dmaArray[i].setBus(this);
	} 
	keysStatus = 0xFFFF;
}

u8* Bus::writeIo(u32 adress, u32 data) {
	if (adress == 0x04000202) {
		intCtrl.regs.if_ &= ~(u16)data;
		return (u8*)&potHole;
	}
	else {
		return ioAccess(adress);
	}
}

u8* Bus::ioAccess(u32 add) {
	if (add <= 0x04000056) {
		return ppu->readIO(add);
	}
	else if ((add >= DMA_FIRST_MAP_ADDRESS) && (add <= DMA_LAST_MAP_ADDRESS)) {
		u8 dmaNb = Dma::selectDma(add);
		printf("accessed dma %d\n", dmaNb);
		return (u8*)dmaArray[dmaNb].readIO((add & 0xFF) - 0xB0 - 0xC * dmaNb);
	}
	else if ((add >= TIMER_FIRST_ADRESS) && (add <= TIMER_LAST_ADRESS)) {
		printf("timer access attempt!\n");
		return (u8*)&potHole;
	}
	else if (add == 0x4000130) {
		return (u8*)&keysStatus;
	}
	else if (add == 0x4000134) {
		potHole = 0;
		return (u8*)&potHole;
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
	return (u8*)&potHole;
}

u8* Bus::getMemoryChunkFromAddress(u32 add) {
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
		return vram + (add & 0x0001FFFF);//TODO: check mirroring 
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
		printf("Invalid memory access: %08x\n", add);
		return nullptr;
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

u8* Bus::get8bitWritableChunk(u32 add) {
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
		printf("Invalid memory access: %08x\n", add);
		return nullptr;
	}
}

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
		return *(ioAccess(addr));
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
		return *(u16*)(ioAccess(addr));
	case 0x05000000:
		return *(u16*)(palette_ram + (addr & 0x000003FF));
	case 0x06000000:
		return *(u16*)(vram + (addr & 0x0001FFFF)); /*TODO: check mirroring*/
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
		return *(u32*)(bios + addr);
	case 0x02000000:
		return *(u32*)(ewram + (addr & 0x0003FFFF));
	case 0x03000000:
		return *(u32*)(iwram + (addr & 0x00007FFF));
	case 0x04000000:
		return *(u32*)(ioAccess(addr));
	case 0x05000000:
		return *(u32*)(palette_ram + (addr & 0x000003FF));
	case 0x06000000:
		return *(u32*)(vram + (addr & 0x0001FFFF)); /*TODO: check mirroring*/
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
		*(u16*)(writeIo(addr, data)) = data;
		break;
	case 0x05000000:
		*(u16*)(palette_ram + (addr & 0x000003FF)) = data;
		break;
	case 0x06000000:
		*(u16*)(vram + (addr & 0x0001FFFF)) = data;
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
		*(u32*)(writeIo(addr2, data)) = data;
		break;
	case 0x05000000:
		*(u32*)(palette_ram + (addr2 & 0x000003FF)) = data;
		break;
	case 0x06000000:
		*(u32*)(vram + (addr2 & 0x0001FFFF)) = data;
		break;
	case 0x07000000:
		*(u32*)(oam + (addr2 & 0x000003FF)) = data;
		break;
	default:
		printf("Invalid memory access when writing 32 bits: %08x\n", addr);
		break;
	}
}

void Bus::loadGamePack(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	if (fp == nullptr) {
		printf("Failed to open file %s\n", filename);
		exit(1);
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
