#include "ppu/ppu.h"
//include a clock to measure the time taken by the function
#include <ctime>
#define LINE_SIZE_IN_DRAWN_PX SCREEN_WIDTH
#define NUMBER_OF_DRAWN_LINES SCREEN_HEIGHT

#define UNDRAWN_LINES 68
#define UNDRAWN_COLUMNS 68

#define REAL_HBLANK_CYCLES (LINE_SIZE_IN_DRAWN_PX + UNDRAWN_COLUMNS)
#define REAL_VBLANK_CYCLES (NUMBER_OF_DRAWN_LINES + UNDRAWN_LINES)

#define DISPSTAT_HBLANK_FLAG 0x0002
#define DISPSTAT_VBLANK_FLAG 0x0001
#define DISPCNT_MODE_MASK 0x0007
#define DISPSTAT_VCOUNT_MATCH_FLAG 0x0004
#define IN_VDRAW_AREA ((cycle < SCREEN_WIDTH) && (scanline < SCREEN_HEIGHT))

Ppu::Ppu(Screen* s, Bus* bus) : lcd{ 0 }, bus{ bus }, screen{ s } {
	this->pixels = screen->getPixels();

}

static u8 dummy = 0;
u8* Ppu::readIO(u32 addr) {
	//return (lcd.array + (addr & 0xFF));
	if ((addr < 0x04000010) || (addr >= 0x04000048)) {
		return (lcd.array + (addr & 0xFF));
	}
	else {
		printf("Invalid PPU read at address %08x\n", addr);
		return &dummy;
	}
}

void Ppu::writeIO32(u32 addr, u32 data) {
	*(u32*)(lcd.array + (addr & 0xFF)) = data;
}

void Ppu::writeIO16(u32 addr, u16 data) {
	*(u16*)(lcd.array + (addr & 0xFF)) = data;
}

void Ppu::writeIO8(u32 addr, u8 data) {
	*(u8*)(lcd.array + (addr & 0xFF)) = data;
}

void Ppu::raiseHBlankIrqIfNeeded() {
	if (lcd.regs.dispstat & 0x0010) {
		bus->intCtrl.regs.if_ |= 0x0002;
	}
}

void Ppu::raiseVBlankIrqIfNeeded() {
	if ((lcd.regs.dispstat & 0x0008)) {
		bus->intCtrl.regs.if_ |= 0x0001;
	}
}

void Ppu::raiseVCountIrqIfNeeded() {
	if ((lcd.regs.dispstat & 0x0020)) {
		bus->intCtrl.regs.if_ |= 0x0004; //VCount
	}
}

void Ppu::updateDispstatAndVCount() {

	u32 dispstatAndVcount = lcd.regs.dispstat;
	if (cycle == 0) {
		if (scanline == (lcd.regs.dispstat >> 8)) {
			dispstatAndVcount |= DISPSTAT_VCOUNT_MATCH_FLAG;
			raiseVCountIrqIfNeeded();
		}
		else {
			dispstatAndVcount &= ~DISPSTAT_VCOUNT_MATCH_FLAG;
		}
	}
	//printf("cycle: %04x\n", cycle);
	if (cycle > (LINE_SIZE_IN_DRAWN_PX - 1)) {
		dispstatAndVcount |= DISPSTAT_HBLANK_FLAG;
		// no H - Blank interrupts are generated within V - Blank period.
		if (cycle == LINE_SIZE_IN_DRAWN_PX && (!(lcd.regs.dispstat & DISPSTAT_VBLANK_FLAG))) {
			raiseHBlankIrqIfNeeded();
		}
	}
	else {
		dispstatAndVcount &= ~DISPSTAT_HBLANK_FLAG;
	}

	if (scanline > (NUMBER_OF_DRAWN_LINES - 1)) {
		dispstatAndVcount |= DISPSTAT_VBLANK_FLAG;
		if (scanline == (NUMBER_OF_DRAWN_LINES) && (cycle == 0)) {
			raiseVBlankIrqIfNeeded();
		}
	}
	else {
		dispstatAndVcount &= ~DISPSTAT_VBLANK_FLAG;
	}

	u32 currentScanline = scanline << 16; //to be put in DISPSTAT
	dispstatAndVcount = (dispstatAndVcount & 0xF0FF) | currentScanline;

	lcd.regs.dispstat = dispstatAndVcount;
}


void Ppu::tick() {
	static u8 frameCounter = 0;
	//clock init
	static clock_t start = clock();
	if (IN_VDRAW_AREA) {
		switch (lcd.regs.dispcnt & DISPCNT_MODE_MASK) {
		case 0x0:
		case 0x1://printf("Mode 1\n");break;
		case 0x2://printf("Mode 2\n");break;
			if (cycle == 1)mode0Orchestrator(nullptr); break;
		case 0x3:if (cycle == 1)mode3(); break;
		case 0x4:if (cycle == 1)mode4(); break;
		case 0x5:if (cycle == 1 && scanline < 128)mode5(); break;
		default:break; // not supposed to happen
		}
		obj();
	}
	cycle++;
	if (cycle == REAL_HBLANK_CYCLES) {
		cycle = 0;
		scanline++;
		lcd.regs.vcount++;
	}
	if (scanline == (REAL_VBLANK_CYCLES - 1)) {
		lcd.regs.vcount = 0;
		screen->updateScreen();
		frameCounter++;
		if (frameCounter == 60) {
			printf("FPS: %f\n", (double)CLOCKS_PER_SEC / (clock() - start) * 60);
			frameCounter = 0;
			start = clock();
		}
		memset(pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));
		scanline = 0;
	}

	updateDispstatAndVCount();
}
