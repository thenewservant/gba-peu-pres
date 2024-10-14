#include "common/types.h"
#include "timer/timer.h"

#define TIMER_FIRST_ADRESS 0x04000100
#define TIMER_LAST_ADRESS 0x0400010F

class Bus;

class TimerManager {
private:
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