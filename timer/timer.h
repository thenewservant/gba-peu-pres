#ifndef TIMER_H
#define TIMER_H

#include "../common/types.h"

enum PRESCALE_SELECTOR {
	F_1 = 0,
	F_64 = 1,
	F_256 = 2,
	F_1024 = 3
};

class Timer {
private:
	u8 timerId;
	Timer* nextTimer;
public:
	u16 reloadValue;
	u16 counter; // what value is the counter currently at
	u8 prescalerSelection; // 2 bits
	u16 prescalerTemp; // enoug to count up to 1024
	bool countUpTiming; // false = normal, true = tick from previous timer
	bool timerIrqEnable; // false = disable, true = enable
	bool timerEnable; // false = stop, true = operate
	bool didOverflow; // true if the timer overflowed just then
public:
	void tick();
	void tickNextTimer(); // to be called by the next timer in the chain
	void tickFromPreviousTimer();
	u16 getCounter();
	u16 getControl();
	void setReload(u16 data);
	void setControl(u16 data);
	void enable(bool value);
	Timer(u8 id, Timer* nextTimer);
};

#endif
