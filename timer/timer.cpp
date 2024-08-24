#include "timer.h"

Timer::Timer(u8 timerId, Timer* nextTimer) {
	this->nextTimer = nextTimer;
	this->timerId = timerId;

	this->reloadValue = 0;
	this->counter = 0;
	this->prescalerSelection = 0;
	this->countUpTiming = false;
	this->timerIrqEnable = false;
	this->timerStartStop = false;
	this->prescalerSelection = 0;
}

void Timer::tick() {
	if (!this->timerStartStop || this->countUpTiming) {
		return;
	}
	switch (prescalerSelection) {
	case 0:
		counter++;
		break;
	case 1:
		if (prescalerTemp == 64) {
			counter++;
			prescalerTemp = 0;
		}
		else {
			prescalerTemp++;
		}
		break;
	case 2:
		if (prescalerTemp == 256) {
			counter++;
			prescalerTemp = 0;
		}
		else {
			prescalerTemp++;
		}
		break;
	case 3:
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
	if (counter == reloadValue) {
		didOverflow = true;
		counter = 0;
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
		this->counter++;
	}
}