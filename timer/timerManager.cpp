#include "timermanager.h"

Timer TimerManager::timers[4] = { Timer(0, &timers[1]), Timer(1, &timers[2]), Timer(2, &timers[3]), Timer(3, nullptr) };

void TimerManager::tickTimers() {
	for (u8 i = 0; i < 4; i++) {
		timers[i].tick();
		timers[i].tickNextTimer();
	}
}

void TimerManager::write(u32 addr, u32 data) {
	switch (addr & 0x2) {
	case 0x0:
		setTimerControl(addr & 0x3, (u16) data);
		break;
	case 0x2:
		setTimerReload(addr & 0x3, (u16) data);
		break;
	}
}

void TimerManager::setTimerReload(u8 timerId, u16 data) {
	this->timers[timerId].reloadValue = data;
}

void TimerManager::setTimerControl(u8 timerId, u16 data) {
	this->timers[timerId].prescalerSelection = data & 0x3;
	this->timers[timerId].countUpTiming = (data & 0x4) != 0;
	this->timers[timerId].timerIrqEnable = (data & 0x40) != 0;
	this->timers[timerId].timerStartStop = (data & 0x80) != 0;
}