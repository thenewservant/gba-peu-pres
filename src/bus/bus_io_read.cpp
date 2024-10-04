#include "bus/gba_bus.h"
#include "ppu/ppu.h"

u32 Bus::ioRead32(u32 addr) {
	if (addr <= 0x04000056) {
		return *(u32*)ppu->readIO(addr);
	}
	else if ((addr >= DMA_FIRST_MAP_ADDRESS) && (addr <= DMA_LAST_MAP_ADDRESS)) {
		return this->dmaManager->read32(addr);
	}
	else if ((addr >= TIMER_FIRST_ADRESS) && (addr <= TIMER_LAST_ADRESS)) {
		return timerManager->read32(addr);
	}
	else if (addr == 0x4000130) {
		return (u32)keysStatus;
	}
	else if (addr >= 0x4000200) {
		u16 addr2 = addr & 0xF00;
		if (addr2 == 0x200) {
			return *(u32*)(intCtrl.array + (addr2 & 0x000000FF));
		}
		else if (addr2 == 0x300) {
			return *(u32*)(pwrStatus.array + (addr2 & 0x000000FF));
		}
		else if (addr2 == 0x400) { // supposedly a bug, unused and undocumented access
			return potHole;
		}
		else if ((addr2 & 0x0F00FFFF) == 0x04000800) {
			return internalMemoryControl;
		}
	}
}

u16 Bus::ioRead16(u32 addr) {
	if (addr <= 0x04000056) {
		return *(u16*)ppu->readIO(addr);
	}
	else if ((addr >= DMA_FIRST_MAP_ADDRESS) && (addr <= DMA_LAST_MAP_ADDRESS)) {
		return this->dmaManager->read16(addr);
	}
	else if ((addr >= TIMER_FIRST_ADRESS) && (addr <= TIMER_LAST_ADRESS)) {
		return timerManager->read16(addr);
	}
	else if (addr == 0x4000130) {
		return (u16)keysStatus;
	}
	else if (addr >= 0x4000200) {
		u16 addr2 = addr & 0xF00;
		if (addr2 == 0x200) {
			return *(u16*)(intCtrl.array + (addr2 & 0x000000FF));
		}
		else if (addr2 == 0x300) {
			return *(u16*)(pwrStatus.array + (addr2 & 0x000000FF));
		}
		else if (addr2 == 0x400) { // supposedly a bug, unused and undocumented access
			return (u16)potHole;
		}
		else if ((addr2 & 0x0F00FFFF) == 0x04000800) {
			return (u16)internalMemoryControl;
		}
	}
}

u8 Bus::ioRead8(u32 addr) {
	if (addr <= 0x04000056) {
		return *ppu->readIO(addr);
	}
	else if ((addr >= DMA_FIRST_MAP_ADDRESS) && (addr <= DMA_LAST_MAP_ADDRESS)) {
		return this->dmaManager->read8(addr);
	}
	else if ((addr >= TIMER_FIRST_ADRESS) && (addr <= TIMER_LAST_ADRESS)) {
		return timerManager->read8(addr);
	}
	else if (addr == 0x4000130) {
		return (u8)keysStatus;
	}
	else if (addr >= 0x4000200) {
		u16 addr2 = addr & 0xF00;
		if (addr2 == 0x200) {
			return *(intCtrl.array + (addr2 & 0x000000FF));
		}
		else if (addr2 == 0x300) {
			return *(pwrStatus.array + (addr2 & 0x000000FF));
		}
		else if (addr2 == 0x400) { // supposedly a bug, unused and undocumented access
			return (u8)potHole;
		}
		else if ((addr2 & 0x0F00FFFF) == 0x04000800) {
			return (u8)internalMemoryControl;
		}
	}
}