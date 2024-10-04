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

enum DMA_FIELDS {
	DMA_SOURCE_ADRESS_0 = 0,
	DMA_SOURCE_ADRESS_1,
	DMA_SOURCE_ADRESS_2,
	DMA_SOURCE_ADRESS_3,
	DMA_DESTINATION_ADRESS_0,
	DMA_DESTINATION_ADRESS_1,
	DMA_DESTINATION_ADRESS_2,
	DMA_DESTINATION_ADRESS_3,
	DMA_WORD_COUNT_0,
	DMA_WORD_COUNT_1,
	DMA_CONTROL_0,
	DMA_CONTROL_1
};

class Bus;

class DmaManager {
private:
	static Dma dmas[4];

	void writeControl1(u8 dmaId, u8 data);
	void writeControl0(u8 dmaId, u8 data);
public:
public:
	DmaManager(Bus* bus);
	void tick();

	void write32(u32 addr, u32 data);
	u8 readControl0(u8 dmaId);
	void write16(u32 addr, u16 data);
	void write8(u32 addr, u8 data);

	u32 read32(u32 addr);
	u16 read16(u32 addr);
	u8 readControl1(u8 dmaId);
	u8 read8(u32 addr);
};

#endif // DMA_MANAGER_H