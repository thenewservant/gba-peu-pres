#include "ppu/ppu.h"

void Ppu::mode3()
{
	u16 value = bus->read16(0x06000000 + (scanline * SCREEN_WIDTH + cycle) * 2);
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
