#include "ppu/ppu.h"

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

Ppu::Ppu(Screen* s, Bus* bus) : lcd{ 0 }, bus{ bus }, screen{ s } {}

u8* Ppu::readIO(u32 addr) {
	return (lcd.array + (addr & 0xFF));
}

u8 Ppu::readIOValue(u32 addr) {
	return lcd.array[addr & 0xFF];
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
	//if (lcd.regs.dispstat & DISPSTAT_HBLANK_FLAG) {
		if (lcd.regs.dispstat & 0x0010) {
			bus->intCtrl.regs.if_ |= 0x0002;
		}
	//}
}

void Ppu::raiseVBlankIrqIfNeeded() {
	//if (lcd.regs.dispstat & DISPSTAT_VBLANK_FLAG) {
		if ((lcd.regs.dispstat & 0x0008)) {
			bus->intCtrl.regs.if_ |= 0x0001;
		}
	//}
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
		if (scanline == (NUMBER_OF_DRAWN_LINES ) && (cycle == 0)) {
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


void Ppu::mode3()
{
	u16 value = bus->read16(0x06000000 + (scanline * SCREEN_WIDTH + cycle)*2);
	u32 r = (value & 0x1F) << 3;
	u32 g = ((value & 0x3E0) >> 5) << 3;
	u32 b = ((value & 0x7C00) >> 10) << 3;
	u32 rgb = (r << 24) | (g << 16) | (b << 8);
	screen->getPixels()[scanline * SCREEN_WIDTH + cycle] = rgb;
}

void Ppu::mode4()
{	
	u16 pxPosInScreen = (scanline * SCREEN_WIDTH + cycle);
	u32 vramBase = (0x06000000 | ((lcd.regs.dispcnt & BIT(4)) ? 0xA000 : 0));
	u8 paletteAddress = bus->read8VRAM(vramBase + pxPosInScreen);
	u16 paletteValue = bus->read16Palette(0x05000000 + paletteAddress * 2);
	u32 r = (paletteValue & 0x1F) << 3;
	u32 g = ((paletteValue & 0x3E0) >> 5) << 3;
	u32 b = ((paletteValue & 0x7C00) >> 10) << 3;
	u32 rgb = (r << 24) | (g << 16) | (b << 8);
	screen->getPixels()[scanline * SCREEN_WIDTH + cycle] = rgb;
}

void Ppu::mode5()
{
	//todo
}


void Ppu::tick() {
	if (IN_VDRAW_AREA) {
		switch (lcd.regs.dispcnt & DISPCNT_MODE_MASK) {
		case 0x0:mode0(); break;
		case 0x1://printf("Mode 1\n");break;
		case 0x2://printf("Mode 2\n");break;
		case 0x3:mode3();break;
		case 0x4:mode4();break;
		case 0x5:mode5();break;
		default:break; // not supposed to happen
		}
		//obj();
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
		scanline = 0;
	}
	
	updateDispstatAndVCount();
}
