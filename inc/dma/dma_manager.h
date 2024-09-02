#ifndef DMA_MANAGER_H
#define DMA_MANAGER_H

#include "dma/dma.h"


#define DMA_FIRST_MAP_ADDRESS 0x040000B0
#define DMA_SPACE_SIZE 0xC // how many bytes a single DMA occupies in memory

enum DMA_DEST_ADJUSTMENT {
	DMA_DST_INC = 0,
	DMA_DST_DEC,
	DMA_DST_FIXED,
	DMA_DST_RELOAD
};

class Bus;

class DmaManager {
private:
	static Dma dmas[4];
public:
	DmaManager(Bus* bus);
	void write32(u32 addr, u32 data);
	void write16(u32 addr, u16 data);
	void write8(u32 addr, u8 data);

	u32 read32(u32 addr);
	u16 read16(u32 addr);
	u8 read8(u32 addr);

};

#endif // DMA_MANAGER_H