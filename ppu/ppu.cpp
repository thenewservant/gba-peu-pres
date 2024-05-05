#include "ppu.h"

#define LINE_SIZE_IN_DRAWN_PX SCREEN_WIDTH
#define NUMBER_OF_DRAWN_LINES SCREEN_HEIGHT

#define UNDRAWN_LINES 68
#define UNDRAWN_COLUMNS 68

#define REAL_HBLANK_CYCLES (LINE_SIZE_IN_DRAWN_PX + UNDRAWN_COLUMNS)
#define REAL_VBLANK_CYCLES (NUMBER_OF_DRAWN_LINES + UNDRAWN_LINES)

#define DIPSTAT_HBLANK_FLAG 0x0002
#define DIPSTAT_VBLANK_FLAG 0x0001
#define DISPCNT_MODE_MASK 0x0007

#define IN_VDRAW_AREA ((cycle < SCREEN_WIDTH) && (scanline < SCREEN_HEIGHT))

Ppu::Ppu(Screen* s, Bus* bus) : lcd{ 0 }, bus{ bus }, screen{ s } {}

u8* Ppu::readIO(u32 addr) {
	return (lcd.array + (addr & 0xFF));
}

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
	u16 value = bus->read16(0x05000000 + bus->read8((0x06000000 | ((lcd.regs.dispcnt & BIT(4))?0xA000:0)) + (scanline * SCREEN_WIDTH + cycle)) * 2);
	u32 r = (value & 0x1F) << 3;
	u32 g = ((value & 0x3E0) >> 5) << 3;
	u32 b = ((value & 0x7C00) >> 10) << 3;
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
		case 0x0://printf("Mode 0\n");break;
		case 0x1://printf("Mode 1\n");break;
		case 0x2://printf("Mode 2\n");break;
		case 0x3:mode3(); break;
		case 0x4:mode4();break;
		case 0x5:mode5();break;
		default:break; // not supposed to happen
		}
	}
	cycle++;
	if (cycle == REAL_HBLANK_CYCLES) {
		cycle = 0;
		scanline++;
	}
	if (scanline == (REAL_VBLANK_CYCLES - 1)) {
		screen->updateScreen();
		scanline = 0;
	}

	updateDipstat();
}

