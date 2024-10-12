#pragma once
#ifndef PPU_BITMAP_MODES_H
#define PPU_BITMAP_MODES_H
#include "ppu/ppu.h"

constexpr u32 bitmapColorDecoder(uint16_t value) {
	u32 r = (value & 0x1F) << 3;        //red component
	u32 g = ((value & 0x3E0) >> 5) << 3; //green component
	u32 b = ((value & 0x7C00) >> 10) << 3; //blue component
	return (r << 24) | (g << 16) | (b << 8);
}

constexpr size_t LUT_SIZE = 65536;

constexpr u32 generateLUT(u32 lut[LUT_SIZE], uint16_t index) {
	for (size_t index = 0; index < LUT_SIZE; ++index) {
		lut[index] = bitmapColorDecoder(index);
	}
	return 0;
}

static u32 bitmapLut[LUT_SIZE] = {};
static const u32 dummy = generateLUT(bitmapLut, 0);

#endif // !PPU_BITMAP_MODES_H
