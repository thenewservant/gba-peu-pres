#ifndef DMA_H
#define DMA_H

#include "../common/types.h"

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



//The most significant address bits are ignored, only the least significant 27 or 28 bits are used
//(max 07FFFFFFh internal memory, or max 0FFFFFFFh any memory)
enum DMASAD {
	DMA0SAD = 0x040000B0, //internal memory only
	DMA1SAD = 0x040000BC, //all
	DMA2SAD = 0x040000C8, //all
	DMA3SAD = 0x040000D4  //all
};

enum DMADAD {
	DMA0DAD = 0x040000B4, //internal memory only
	DMA1DAD = 0x040000C0, //internal memory only
	DMA2DAD = 0x040000CC, //internal memory only
	DMA3DAD = 0x040000D8  //all
};

//DMA word count
enum DMAWCNT {
	DMA0CNT_L = 0x040000B8, //14 bit, 1..4000h
	DMA1CNT_L = 0x040000C4, //14 bit, 1..4000h
	DMA2CNT_L = 0x040000D0, //14 bit, 1..4000h
	DMA3CNT_L = 0x040000DC  //16 bit, 1..10000h
};

//40000BAh - DMA0CNT_H - DMA 0 Control(R / W)
//40000C6h - DMA1CNT_H - DMA 1 Control(R / W)
//40000D2h - DMA2CNT_H - DMA 2 Control(R / W)
//40000DEh - DMA3CNT_H - DMA 3 Control(R / W)

enum DMA_CONTROL_AD {
	DMA0CNT_H = 0x040000BA,
	DMA1CNT_H = 0x040000C6,
	DMA2CNT_H = 0x040000D2,
	DMA3CNT_H = 0x040000DE
};

enum DMA_DEST_ADJUSTMENT {
	DMA_DST_INC = 0,
	DMA_DST_DEC,
	DMA_DST_FIXED,
	DMA_DST_RELOAD
};

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