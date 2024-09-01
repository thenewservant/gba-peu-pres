#include "bus/gba_bus.h"
#include "ppu/ppu.h"

// TODO : check if u8 casts for 32 and 16 bits are adequate

void Bus::ioWrite32(u32 addr, u32 data) {
	if (addr <= 0x04000056) {
		ppu->writeIO32(addr, data);
	}
	else if ((addr >= DMA_FIRST_MAP_ADDRESS) && (addr <= DMA_LAST_MAP_ADDRESS)) {
		u8 dmaNb = Dma::selectDma(addr);
		printf("accessed dma %d\n", dmaNb);
		//dmaArray[dmaNb].writeIO32((addr & 0xFF) - 0xB0 - 0xC * dmaNb, data);
	}
	else if ((addr >= TIMER_FIRST_ADRESS) && (addr <= TIMER_LAST_ADRESS)) {
		timerManager->write32(addr, data);
	}
	else if (addr == 0x4000130) {
		 keysStatus = (u16)data;
	}
	else if (addr >= 0x4000200) {
		u16 addr2 = addr & 0xF00;
		if (addr == 0x04000202) {
			intCtrl.regs.if_ &= ~(u16)data;
		}
		else if (addr2 == 0x200) {
			intCtrl.array[(addr2 & 0x000000FF)] = (u8)data;
		}
		else if (addr2 == 0x300) {
			pwrStatus.array[(addr2 & 0x000000FF)] = (u8)data;
		}
		else if (addr2 == 0x400) { // supposedly a bug, unused and undocumented access
			//return potHole;
		}
		else if ((addr2 & 0x0F00FFFF) == 0x04000800) {
			internalMemoryControl = (u8)data;
		}
	}
}


void Bus::ioWrite16(u32 addr, u16 data) {
	if (addr <= 0x04000056) {
		ppu->writeIO16(addr, data);
	}
	else if ((addr >= DMA_FIRST_MAP_ADDRESS) && (addr <= DMA_LAST_MAP_ADDRESS)) {
		u8 dmaNb = Dma::selectDma(addr);
		printf("accessed dma %d\n", dmaNb);
		//dmaArray[dmaNb].writeIO16((addr & 0xFF) - 0xB0 - 0xC * dmaNb, data);
	}
	else if ((addr >= TIMER_FIRST_ADRESS) && (addr <= TIMER_LAST_ADRESS)) {
		timerManager->write16(addr, data);
	}
	else if (addr == 0x4000130) {
		keysStatus = (u16)data;
	}
	else if (addr >= 0x4000200) {
		u16 addr2 = addr & 0xF00;
		if (addr2 == 0x200) {
			intCtrl.array[(addr2 & 0x000000FF)] = (u8)data;
		}
		else if (addr2 == 0x300) {
			pwrStatus.array[(addr2 & 0x000000FF)] = (u8)data;
		}
		else if (addr2 == 0x400) { // supposedly a bug, unused and undocumented access
			//return potHole;
		}
		else if ((addr2 & 0x0F00FFFF) == 0x04000800) {
			internalMemoryControl = (u8)data;
		}
	}
}