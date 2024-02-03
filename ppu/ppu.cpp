#include "ppu.h"


Ppu::Ppu(Screen* s, Bus* bus) {
	this->ppuRegs = (u8*)calloc(0x57, sizeof(u8));
	
	printf("ppu regs address in memory: %p, %p\n", ppuRegs, &ppuRegs);
	screen = s;
	this->bus = bus;
	
}

u8* Ppu::readIO(u32 addr) {
	return (ppuRegs + (addr & 0xFF));
}

#define REAL_HBLANK_CYCLES SCREEN_WIDTH + 68
#define REAL_VBLANK_CYCLES SCREEN_HEIGHT + 68

void Ppu::tick() {

	if ((cycle < SCREEN_WIDTH) && (scanline < SCREEN_HEIGHT)){


		if (true) {//mode 4
			u16 value = bus->read16(0x05000000 + bus->read8(0x06000000 + (scanline * SCREEN_WIDTH + cycle) * 2));
			u8 r = (value & 0x1F) <<3;
			u8 g = ((value & 0x3E0) >> 5)<<3;
			u8 b =  ((value & 0x7C00) >> 10)<<3;
			u32 rgb = (r << 24) | (g << 16) | (b<<8);
			screen->getPixels()[scanline * SCREEN_WIDTH + cycle] = rgb;
		}
	}


	
	cycle++;
	if (cycle == REAL_HBLANK_CYCLES) {
		
		cycle = 0;
		scanline++;
	}
	if (scanline == REAL_VBLANK_CYCLES) {
		screen->updateScreen();
		scanline = 0;
	}

}