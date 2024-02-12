#include "ppu.h"


Ppu::Ppu(Screen* s, Bus* bus) {
	printf("ppu lcd address in memory: %p, %p\n", lcd, &lcd);
	screen = s;
	this->bus = bus;

}

u8* Ppu::readIO(u32 addr) {
	return (lcd.array + (addr & 0xFF));
}

#define LINE_SIZE_IN_DRAWN_PX SCREEN_WIDTH
#define NUMBER_OF_DRAWN_LINES SCREEN_HEIGHT

#define UNDRAWN_LINES 68
#define UNDRAWN_COLUMNS 68

#define REAL_HBLANK_CYCLES (LINE_SIZE_IN_DRAWN_PX + UNDRAWN_COLUMNS)
#define REAL_VBLANK_CYCLES (NUMBER_OF_DRAWN_LINES + UNDRAWN_LINES)

#define DIPSTAT_HBLANK_FLAG 0x0002
#define DIPSTAT_VBLANK_FLAG 0x0001

void Ppu::updateDipstat() {

	u32 dipstatAndVcount = 0;

	//printf("cycle: %04x\n", cycle);
	if (cycle > (LINE_SIZE_IN_DRAWN_PX - 1)) {
		
		dipstatAndVcount |= DIPSTAT_HBLANK_FLAG;
	}
	else {
		dipstatAndVcount &= ~DIPSTAT_HBLANK_FLAG;
	}

	if (scanline > (NUMBER_OF_DRAWN_LINES - 1)) {
		dipstatAndVcount |= DIPSTAT_VBLANK_FLAG;
	}
	else {
		dipstatAndVcount &= ~DIPSTAT_VBLANK_FLAG;
	}

	u32 currentScanline = scanline << 16; //to be put in DIPSTAT
	//printf("currentScanline: %04x\n", scanline);
	dipstatAndVcount = (dipstatAndVcount & 0xF0FF) | currentScanline;


	lcd.regs.dispstat = dipstatAndVcount;
}


void Ppu::tick() {

	if ((cycle < SCREEN_WIDTH) && (scanline < SCREEN_HEIGHT)) {
		if (true) {//mode 4
			u16 value = bus->read16(0x05000000 + bus->read8(0x06000000 + (scanline * SCREEN_WIDTH + cycle)) * 2);
			u32 r = (value & 0x1F) << 3;
			u32 g = ((value & 0x3E0) >> 5) << 3;
			u32 b = ((value & 0x7C00) >> 10) << 3;
			u32 rgb = (r << 24) | (g << 16) | (b << 8);
			
			screen->getPixels()[scanline * SCREEN_WIDTH + cycle] = rgb;
		}
	}

	cycle++;
	if (cycle == REAL_HBLANK_CYCLES) {
		cycle = 0;
		scanline++;
	}
	if (scanline == (REAL_VBLANK_CYCLES-1)) {
		screen->updateScreen();
		scanline = 0;
	}

	updateDipstat();
}

