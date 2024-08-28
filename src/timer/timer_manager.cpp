#include "timer_manager.h"

Timer TimerManager::timers[4] = { Timer(0, &timers[1]), Timer(1, &timers[2]), Timer(2, &timers[3]), Timer(3, nullptr) };

void TimerManager::tick() {
	for (u8 i = 0; i < 4; i++) {
		timers[i].tick();
		timers[i].tickNextTimer();
	}
}


u8 TimerManager::read8(u32 addr) {
	bool isSecondByte = (addr & 0x1) == 1;
	bool isControl = (addr & 0x2) == 1;
	u8 timerId = (addr & 0xC) >> 2;
	if (isControl) {
		return (u8)(timers[timerId].getControl() >> (isSecondByte ? 8 : 0));
	}
	else {
		return (u8)(timers[timerId].getCounter() >> (isSecondByte ? 8 : 0));
	}
}

u16 TimerManager::read16(u32 addr) {
	return (u16)(read8(addr) | (read8(addr + 1) << 8));
}

u32 TimerManager::read32(u32 addr) {
	return (u32)(read16(addr) | (read16(addr + 2) << 16));
}

void TimerManager::write8(u32 addr, u8 data) {
	printf("Writing 8 bits to %x: %x to timer %d\n", addr & 0xFFFF, data, (addr & 0xC) >> 2);
	bool isSecondByte = (addr & 0x1) == 1;
	bool isControl = (addr & 0x2) == 0x2;
	u8 timerId = (addr & 0xC) >> 2;
	if (isControl) {
		u16 control = timers[timerId].getControl();
		control &= isSecondByte ? 0x00FF : 0xFF00;
		control |= data << (isSecondByte ? 8 : 0);
		printf("control to be assigned to timer %d: %x\n", timerId, control);
		timers[timerId].setControl(control);
	}
	else {
		u16 counter = timers[timerId].getCounter();
		counter &= isSecondByte ? 0x00FF : 0xFF00;
		counter |= data << (isSecondByte ? 8 : 0);
		timers[timerId].setReload(counter);
	}
}

void TimerManager::write16(u32 addr, u16 data) {
	write8(addr, (u8)data);
	write8(addr + 1, (u8)(data >> 8));
}

void TimerManager::write32(u32 addr, u32 data) {
	write16(addr, (u16)data);
	write16(addr + 2, (u16)(data >> 16));
}