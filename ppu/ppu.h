#ifndef PPU_H
#define PPU_H
#include "../common/types.h"
#include "../screen/screen.h"
#include "../bus/gba_bus.h"

class Ppu {
private:
	Bus* bus;
	Screen* screen;
	
	u16 scanline = 0;
	u16 cycle = 0;
public:
	u8* ppuRegs;
	Ppu(Screen* screen, Bus* bus);
	u8* readIO(u32 addr);
	void tick();
};

#endif