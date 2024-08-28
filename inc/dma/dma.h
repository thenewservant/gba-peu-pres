#ifndef DMA_H
#define DMA_H

#include "common/types.h"

#define DMA_FIRST_MAP_ADDRESS 0x040000B0
#define DMA_LAST_MAP_ADDRESS 0x040000DE

PACK(struct dmaRegs_t {
	u32 dmasad;
	u32 dmadad;
	u16 dmacnt_l;
	u16 dmacnt_h;
});

typedef union _dmaUnion_t {
	struct dmaRegs_t regs;
	u8 array[sizeof(struct dmaRegs_t)];
} DmaUnion;

#define DMA_ADJ_MASK (BIT(5) | BIT(6))
#define DMA_ADJ_OFFSET 5

enum DMA_SRC_ADJUSTMENT {
	DMA_SRC_INC = 0,
	DMA_SRC_DEC,
	DMA_SRC_FIXED,
};

#define DMA_SRC_MASK (BIT(7) | BIT(8))
#define DMA_SRC_OFFSET 7

#define DMA_REPEAT BIT(9)

enum DMA_CHUNK_SIZE_LENGTH {
	DMA_16 = 0,
	DMA_32 = 1 << 0xA
};

#define DMA_CHUNK_SIZE 0xA

enum DMA_TIMING_MODE {
	DMA_NOW=0,
	DMA_AT_VBLANK,
	DMA_AT_HBLANK,
	DMA_AT_REFRESH
};

#define DMA_TIMING_MASK (BIT(12) | BIT(13))
#define DMA_TIMING_OFFSET 12

#define DMA_IRQ BIT(14)
#define DMA_ENABLE BIT(15)

enum DMA_NB {
	DMA0 = 0,
	DMA1,
	DMA2,
	DMA3
};
class Bus;

class Dma {
private:
	DmaUnion controlRegs;
	Bus* bus;
	enum DMA_NB dmaId;
	u32 dmaSource;
	u32 dmaDest;
	u16 dmaCount;
	u16 dmaControl;

public:
	static enum DMA_NB selectDma(u32 addr);
	u8* readIO(u32 add);
	void notify(u8 signal); // is called by the bus when a signal (HBLANK, VBLANK) is triggered
	void setBus(Bus* bus);
	void transfer();
	Dma(enum DMA_NB dmaId);
	Dma(){}
};


#endif // DMA_H