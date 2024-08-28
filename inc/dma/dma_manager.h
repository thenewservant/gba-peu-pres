#ifndef DMA_MANAGER_H
#define DMA_MANAGER_H

#include "dma/dma.h"


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

class Bus;

class DmaManager {
private:
	Dma* dmaArray[4];
public:
	DmaManager(Bus* bus);
	void writeToDma(u32 addr, u32 data);

};

#endif // DMA_MANAGER_H