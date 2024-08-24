#include "../common/types.h"

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
	bool timerStartStop; // false = stop, true = operate
	bool didOverflow; // true if the timer overflowed just then
public:
	void tick();
	void tickNextTimer(); // to be called by the next timer in the chain
	void tickFromPreviousTimer();
	Timer(u8 id, Timer* nextTimer);
};