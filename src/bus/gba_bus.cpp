#include "bus/gba_bus.h"
#include "ppu/ppu.h"

#pragma warning(disable:4996)

Bus::Bus() {
	this->timerManager = new TimerManager(this);
	this->dmaManager = new DmaManager(this);
	keysStatus = 0xFFFF;
}

u32 vramMirroredAdress(u32 addr) {
	u32 maskedAddr = addr & 0x1FFFF;	
	if (maskedAddr >= 0x18000) {
		return maskedAddr - 0x8000;
	}
	else {
		return maskedAddr;
	}
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
		exit(12344);
		return (u8*)&potHole;/*(u8*)dmaArray[dmaNb].readIO((add & 0xFF) - 0xB0 - 0xC * dmaNb);*/
	}
	else if ((add >= TIMER_FIRST_ADRESS) && (add <= TIMER_LAST_ADRESS)) {
		printf("timer access attempt!\n");
		exit(12345);
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

void Bus::loadGamePack(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	if (fp == nullptr) {
		printf("GamePack loading error: Failed to open file %s\n", filename);
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
		printf("Bios loading error: Failed to open file %s\n", filename);
		return;
	}

	u32 cursor = 0;
	while (!feof(fp)) {
		fread(bios + cursor++, sizeof(u8), 1, fp);
	}
}
