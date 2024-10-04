#include "ppu/ppu.h"
#include <vector>
#include <algorithm>

constexpr u32 VRAM_BASE = 0x06000000;
#define IN_VDRAW_AREA ((cycle < SCREEN_WIDTH) && (scanline < SCREEN_HEIGHT))

#define PALETTE_BASE 0x05000000
#define BG_OFFSET_MASK 0x1FF
#define BG_TILE_INFO_SIZE 2
#define TILE_NUMBER_MASK 0x3FF

#define BG0_PACK_ASSIGN(a,b,c) a=lcd.regs.bg0cnt, b=lcd.regs.bg0hofs, c=lcd.regs.bg0vofs
#define BG1_PACK_ASSIGN(a,b,c) a=lcd.regs.bg1cnt, b=lcd.regs.bg1hofs, c=lcd.regs.bg1vofs
#define BG2_PACK_ASSIGN(a,b,c) a=lcd.regs.bg2cnt, b=lcd.regs.bg2hofs, c=lcd.regs.bg2vofs
#define BG3_PACK_ASSIGN(a,b,c) a=lcd.regs.bg3cnt, b=lcd.regs.bg3hofs, c=lcd.regs.bg3vofs

#define CNT_PRIORITY(x) ((x) & 0x3)
#define IS_BG_ENABLED(x) (((1 << 8) << (x)) & lcd.regs.dispcnt) //x: enum TILE_BG_ID
#define SCREEN_MAP_SIZE 0x800

#define BGXCNT_IS_256_COLORS BIT(7)


enum TEXT_MODE_ENTRYTYPE {
	E256_256 = 0,
	E512_256 = 1,
	E256_512 = 2,
	E512_512 = 3
};


struct sort_by_priority {
	bool operator()(const std::pair<enum TILE_BG_ID, u8>& left, const std::pair<enum TILE_BG_ID, u8>& right) {
		return left.second < right.second;
	}
};

static u32 getRgbFromPaletteAdress(u16 adress) {
	u32 r = (adress & 0x1F) << 3;
	u32 g = ((adress & 0x3E0) >> 5) << 3;
	u32 b = ((adress & 0x7C00) >> 10) << 3;
	return (r << 24) | (g << 16) | (b << 8);
}

void Ppu::mode0SingleLineFeed(u32* line, enum TILE_BG_ID bgType, u16 scanline) {
	u16 cnt=0, hofs=0, vofs=0;
	switch (bgType) {
	case BG0: BG0_PACK_ASSIGN(cnt,hofs, vofs); break;
	case BG1: BG1_PACK_ASSIGN(cnt, hofs, vofs); break;
	case BG2: BG2_PACK_ASSIGN(cnt, hofs, vofs); break;
	case BG3: BG3_PACK_ASSIGN(cnt, hofs, vofs); break;
	}
	
	u32 bgMapDataBase = ((cnt >> 8) & 0x1F); //Base block of the bg1 (tile map)

	bgMapDataBase *= 0x800; //Each block is 2KB
	bgMapDataBase += VRAM_BASE;

	u32 bgTileData = ((cnt >> 2) & 3) * 0x4000; //Base block of the bg1 (tile data)

	u16 scrollX = hofs & BG_OFFSET_MASK;//X Position of the first bg1 pixel
	u16 scrollY = vofs & BG_OFFSET_MASK;//Y Position of the first bg1 pixel

	u8 entryType = (cnt >> 14) & 0x3;

	u8 colAmount = 32;
	u16 cyclicXScroll = 0;
	u16 cyclicYScroll = 0;

	u16 tileInfo = 0;
	bool needToSwitchScreenHorizontal = false;
	bool needToSwitchScreenVertical = false;
	for (int cyc = 0; cyc < SCREEN_WIDTH; cyc++) {

		switch (entryType) {
		case E256_256:
			break;
		case E512_256:
			needToSwitchScreenHorizontal = ((cyc + scrollX) / 256) == 1;
			break;
		case E256_512:
			needToSwitchScreenVertical = ((scanline + scrollY) / 256) == 1;
			break;
		case E512_512:
			needToSwitchScreenHorizontal = ((cyc + scrollX) / 256) == 1;
			needToSwitchScreenVertical = ((scanline + scrollY) / 256) == 1;
			break;
		default:
			break;
		}
		cyclicXScroll = (cyc + scrollX) % 256;
		cyclicYScroll = (scanline + scrollY) % 256;
		tileInfo = bus->read16(bgMapDataBase + (cyclicXScroll / 8) * BG_TILE_INFO_SIZE + (cyclicYScroll / 8) * colAmount * BG_TILE_INFO_SIZE +
			(needToSwitchScreenHorizontal ? SCREEN_MAP_SIZE : 0) +
			(needToSwitchScreenVertical ? SCREEN_MAP_SIZE : 0));

		u8 paletteNumber = (tileInfo >> 12) & 0xF;
		u16 currentTileIndex = tileInfo & TILE_NUMBER_MASK;

		u32 finalColor = 0;
		bool isColor0 = false;
		if (!(cnt & BGXCNT_IS_256_COLORS)) { // 16x16 colors (4bpp)

			u8 currentByteForThisPixel = bus->read8VRAM(bgTileData + 32 * currentTileIndex + ((cyclicXScroll % 8) / 2) + (cyclicYScroll % 8) * 4);
			u32 currentPixelPaletteId = ((currentByteForThisPixel >> ((cyclicXScroll) % 2) * 4) & 0xF);
			finalColor = getRgbFromPaletteAdress(bus->read16Palette(paletteNumber * 16 * 2 + currentPixelPaletteId * 2));
			/*if ((( currentPixelPaletteId /2) & 0x7) == 0) {
				isColor0 = true;
			}*/
		}
		else { // 256 colors(8bpp)
			u8 currentByteForThisPixel = bus->read8VRAM(bgTileData + 64 * currentTileIndex + (cyclicXScroll % 8) + (cyclicYScroll % 8) * 8);
			finalColor = getRgbFromPaletteAdress(bus->read16Palette(currentByteForThisPixel * 2));
			/*if (currentByteForThisPixel == 0) {
				isColor0 = true;
			}*/
		}
		if (!isColor0) { screen->getPixels()[scanline * SCREEN_WIDTH + cyc] = finalColor; }
	}
}

//#pragma optimize("", off)
/*__declspec(noinline) */ void Ppu::mode0() {

	u32 bgMapDataBase = ((lcd.regs.bg0cnt >> 8) & 0x1F); //Base block of the bg0 (tile map)
	
	bgMapDataBase *= 0x800; //Each block is 2KB
	bgMapDataBase += VRAM_BASE;

	u32 bgTileData = ((lcd.regs.bg0cnt >> 2) &3) * 0x4000; //Base block of the bg0 (tile data)

	u16 scrollX = lcd.regs.bg0hofs & BG_OFFSET_MASK;//X Position of the first bg0 pixel
	u16 scrollY = lcd.regs.bg0vofs & BG_OFFSET_MASK;//Y Position of the first bg0 pixel

	u8 entryType = (lcd.regs.bg0cnt >> 14) & 0x3;

	u8 colAmount = 32;
	u16 cyclicXScroll = 0;
	u16 cyclicYScroll = 0;

	u16 tileInfo = 0;
	bool needToSwitchScreenHorizontal = false;
	bool needToSwitchScreenVertical = false;
	switch (entryType) {
	case E256_256:
		break;
	case E512_256:
		needToSwitchScreenHorizontal = ((cycle + scrollX) / 256) == 1;
		break;
	case E256_512:
		needToSwitchScreenVertical = ((scanline + scrollY) / 256) == 1;
		break;
	case E512_512:
		needToSwitchScreenHorizontal = ((cycle + scrollX) / 256) == 1;
		needToSwitchScreenVertical = ((scanline + scrollY) / 256) == 1;
		break;
	default:
		break;
	}
	cyclicXScroll = (cycle + scrollX) % 256;
	cyclicYScroll = (scanline + scrollY) % 256;
	tileInfo = bus->read16(bgMapDataBase + (cyclicXScroll / 8) * BG_TILE_INFO_SIZE + (cyclicYScroll / 8) * colAmount * BG_TILE_INFO_SIZE  +
		(needToSwitchScreenHorizontal ? SCREEN_MAP_SIZE : 0) +
		(needToSwitchScreenVertical ? SCREEN_MAP_SIZE : 0));

	u8 paletteNumber = (tileInfo >> 12) & 0xF;
	u16 currentTileIndex = tileInfo & TILE_NUMBER_MASK;

	u32 finalColor = 0;
	if (!(lcd.regs.bg0cnt & BGXCNT_IS_256_COLORS)) { // 16x16 colors (4bpp)

		u8 currentByteForThisPixel = bus->read8VRAM(bgTileData + 32 * currentTileIndex + ((cyclicXScroll % 8) / 2) + (cyclicYScroll % 8) * 4);
		u32 currentPixelPaletteId = ((currentByteForThisPixel >> ((cyclicXScroll) % 2) * 4) & 0xF);
		finalColor = getRgbFromPaletteAdress(bus->read16Palette(paletteNumber * 16 * 2 + currentPixelPaletteId * 2));

	}
	else { // 256 colors(8bpp)
		u8 currentByteForThisPixel = bus->read8VRAM(bgTileData + 64 * currentTileIndex + (cyclicXScroll % 8) + (cyclicYScroll % 8) * 8);
		finalColor = getRgbFromPaletteAdress(bus->read16Palette(currentByteForThisPixel * 2));
	}
	screen->getPixels()[scanline * SCREEN_WIDTH + cycle] = finalColor;
}

void Ppu::mode0Orchestrator(u32* line) {
	std::vector<std::pair<enum TILE_BG_ID, u8>> bgPriorities;
	static const enum TILE_BG_ID bgTypes[] = { BG0, BG1, BG2, BG3 };
	static const u16 cnts[] = { lcd.regs.bg0cnt, lcd.regs.bg1cnt, lcd.regs.bg2cnt, lcd.regs.bg3cnt };
	
	for (u8 i = 0; i < 4; i++) {
		if (IS_BG_ENABLED(bgTypes[i])) {
			bgPriorities.push_back(std::make_pair(bgTypes[i], CNT_PRIORITY(cnts[i])));
		}
	}

	std::sort(bgPriorities.begin(), bgPriorities.end(), sort_by_priority());
	for (u8 i = 0; i < bgPriorities.size(); i++) {
		mode0SingleLineFeed(line, bgPriorities[i].first, scanline);
	}
}
