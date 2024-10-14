#include "ppu/ppu.h"

#define MODE_5_WIDTH 160

void Ppu::mode3()
{
	u32 scanlineOffset = scanline * SCREEN_WIDTH;

	for (u8 cyc = 0; cyc < SCREEN_WIDTH; cyc++) {
		u32 position = scanlineOffset + cyc;
		u16 value = bus->read16VRAM(position * 2);
		pixels[position] = bitmapLut[value];
	}
}

void Ppu::mode4()
{
	u32 vramBase = ((lcd.regs.dispcnt & BIT(4)) ? 0xA000 : 0);
	u32 scanlineOffset = scanline * SCREEN_WIDTH;
	for (u8 cyc = 0; cyc < SCREEN_WIDTH; cyc++) {
		u32 position = scanlineOffset + cyc;
		u8 paletteAddress = bus->read8VRAM(vramBase + position);
		u16 paletteValue = bus->read16Palette(paletteAddress * 2);
		pixels[position] = bitmapLut[paletteValue];
	}
}

void Ppu::mode5()
{
	u32 vramBase = ((lcd.regs.dispcnt & BIT(4)) ? 0xA000 : 0);
	u32 scanlineScreenOffset = scanline * SCREEN_WIDTH;
	u32 scanlineVramOffset = scanline * MODE_5_WIDTH;
	for (u8 cyc = 0; cyc < MODE_5_WIDTH; cyc++) {
		u32 screenPosition = scanlineScreenOffset + cyc;
		u32 vramPosition = scanlineVramOffset + cyc;
		u16 value = bus->read16VRAM(vramBase + vramPosition * 2);
		pixels[screenPosition] = bitmapLut[value];
	}
}
