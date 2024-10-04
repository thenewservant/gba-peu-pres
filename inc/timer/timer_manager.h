#include "common/types.h"
#include "timer/timer.h"

#define TIMER_FIRST_ADRESS 0x04000100
#define TIMER_LAST_ADRESS 0x0400010F

class Bus;

class TimerManager {
private:

	//0 - 1   Prescaler Selection(0 = F / 1, 1 = F / 64, 2 = F / 256, 3 = F / 1024)
	//	2     Count - up Timing(0 = Normal, 1 = See below); Not used in TM0CNT_H
	//	3 - 5   Not used
	//	6     Timer IRQ Enable(0 = Disable, 1 = IRQ on Timer overflow)
	//	7     Timer Start / Stop(0 = Stop, 1 = Operate)
	static Timer timers[4];

public:
	void tick();
	TimerManager(Bus* bus);
	u8 read8(u32 addr);
	u16 read16(u32 addr);
	u32 read32(u32 addr);
	void write8(u32 addr, u8 data);
	void write16(u32 addr, u16 data);
	void write32(u32 addr, u32 data);
};