#include "ppu.h"

#define IN_VDRAW_AREA ((cycle < SCREEN_WIDTH) && (scanline < SCREEN_HEIGHT))

constexpr u32 VRAM_BASE = 0x06000000;

#define PALETTE_BASE 0x05000000
#define BG_OFFSET_MASK 0x1FF
#define BG_SQUARE_SIZE 256
#define SCREEN_ENTRY_SIZE 2
#define BG_ROW_SIZE 32 * SCREEN_ENTRY_SIZE

#define TILE_NUMBER_MASK 0x3FF

static u32 getRgbFromPaletteAdress(u32 adress) {
	u32 r = (adress & 0x1F) << 3;
	u32 g = ((adress & 0x3E0) >> 5) << 3;
	u32 b = ((adress & 0x7C00) >> 10) << 3;
	return (r << 24) | (g << 16) | (b << 8);
}

void Ppu::mode0() {
	//Starting with BG0

	//BGCNT data
	u32 screenBaseBlock = (lcd.regs.bg0cnt >> 8) & 0x1F; //Base block of the BG0 (tile map)fF
	
	screenBaseBlock *= 0x800; //Each block is 2KB
	screenBaseBlock += VRAM_BASE;

	u32 charBaseBlock = ((lcd.regs.bg0cnt >> 2) &3) * 0x4000; //Base block of the BG0 (tile data)

	u16 scrollX = lcd.regs.bg0hofs & BG_OFFSET_MASK;//X Position of the first BG0 pixel
	u16 scrollY = lcd.regs.bg0vofs & BG_OFFSET_MASK;//Y Position of the first BG0 pixel

	u32 currentTileOffset =  (scanline / 8) * BG_ROW_SIZE + (cycle / 8) * SCREEN_ENTRY_SIZE; //Offset used along with BG_HOFS and VOFS. Depends on current pixel of the screen

	//TILE-specific data
	
	u16 tileData = bus->read16(screenBaseBlock + currentTileOffset);

	//if paletteMode isn't 256
	u8 paletteNumber = (tileData >> 12) & 0xF;
	u16 currentTileIndex = tileData & TILE_NUMBER_MASK;
	//Contains the byte (2pixels in 4bpp mode) for this pixel.
	u32 currentByteForThisPixel = bus->read16(VRAM_BASE + charBaseBlock + 32 * currentTileIndex + ((cycle%8)/2) + (scanline%8) * 4);

	//Most significant nibble of a byte is the left pixel, least significant is the right pixel.
	//Hence (cycle) % 2)*4
	u32 currentPixelPaletteId = ((currentByteForThisPixel >> ((cycle) % 2)*4) & 0xF);
	u32 finalColor = getRgbFromPaletteAdress(bus->read16(PALETTE_BASE + paletteNumber * 16 * 2 + currentPixelPaletteId * 2));
	
	screen->getPixels()[scanline * SCREEN_WIDTH + cycle] = finalColor;
}