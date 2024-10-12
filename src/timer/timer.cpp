#include "timer/timer.h"
#include "bus/gba_bus.h"

Timer::Timer(u8 timerId, Timer* nextTimer) {
	this->nextTimer = nextTimer;
	this->timerId = timerId;

	this->reloadValue = 0;
	this->counter = 0;
	this->prescalerSelection = 0;
	this->countUpTiming = false;
	this->timerIrqEnable = false;
	this->timerEnable = false;
	this->didOverflow = false;
	this->prescalerTemp = 0;
	this->prescalerSelection = 0;
}

void Timer::setBus(Bus* bus) {
	this->bus = bus;
}

void Timer::tick() {
	if (!this->timerEnable || this->countUpTiming) {
		return;
	}
	
	if (counter == 0xFFFF) {
		didOverflow = true;
		counter = reloadValue;
		if (timerIrqEnable) {
			this->bus->intCtrl.regs.if_ |= 1 << (3 + timerId);
		}
		return;
	}
	
	switch (prescalerSelection) {
	case F_1:
		counter++;
		break;
	case F_64:
		if (prescalerTemp == 64) {
			counter++;
			prescalerTemp = 0;
		}
		else {
			prescalerTemp++;
		}
		break;
	case F_256:
		if (prescalerTemp == 256) {
			counter++;
			prescalerTemp = 0;
		}
		else {
			prescalerTemp++;
		}
		break;
	case F_1024:
		if (prescalerTemp == 1024) {
			counter++;
			prescalerTemp = 0;
		}
		else {
			prescalerTemp++;
		}
		break;
	default:
		break;
	}
}

void Timer::tickNextTimer() {
	if (this->didOverflow) {
		this->didOverflow = false;
		this->nextTimer->tickFromPreviousTimer();
	}
}

void Timer::tickFromPreviousTimer() {
	
	if ( this->countUpTiming) {
		if (counter == 0xFFFF) {
			didOverflow = true;
			counter = reloadValue;
			return;
		}
		this->counter++;
	}
}

u16 Timer::getCounter() {
	return this->counter;
}

u16 Timer::getControl() {
	u16 control = 0;
	control |= this->prescalerSelection;
	control |= this->countUpTiming << 2;
	control |= this->timerIrqEnable << 6;
	control |= this->timerEnable << 7;
	return control;
}

void Timer::setReload(u16 val) {
	this->reloadValue = val;
}

void Timer::setControl(u16 val) {
	this->prescalerSelection = val & 0x3;
	this->countUpTiming = (val & 0x4) != 0;
	this->timerIrqEnable = (val & 0x40) != 0;
	this->enable((val & 0x80) != 0);
}

void Timer::enable(bool value) {
	if (!this->timerEnable && value) { 
		this->timerEnable = true;
		this->counter = reloadValue; // when enabled, reset the counter
	}
	else {
		this->timerEnable = value;
	}
}