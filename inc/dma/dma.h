#ifndef DMA_H
#define DMA_H

#include "common/types.h"

#define DMA_FIRST_MAP_ADDRESS 0x040000B0
#define DMA_LAST_MAP_ADDRESS 0x040000DE

#define DMA_TIMING_MASK (BIT(12) | BIT(13))
#define DMA_TIMING_OFFSET 12

#define DMA_IRQ BIT(14)
#define DMA_ENABLE BIT(15)

#define DMA_ADJ_MASK (BIT(5) | BIT(6))
#define DMA_ADJ_OFFSET 5

enum DMA_SRC_ADJUSTMENT {
	DMA_SRC_INC = 0,
	DMA_SRC_DEC,
	DMA_SRC_FIXED,
};

#define DMA_SRC_MASK (BIT(7) | BIT(8))
#define DMA_SRC_OFFSET 7


enum DMA_CHUNK_SIZE_LENGTH {
	DMA_16 = 0,
	DMA_32 = 1 << 0xA
};

#define DMA_CHUNK_SIZE 0xA

enum DMA_NB {
	DMA0 = 0,
	DMA1,
	DMA2,
	DMA3
};

enum DMA_TRANSFER_TYPE {
	DMA_16BIT = 0,
	DMA_32BIT = 1 
};

enum DMA_TIMING_MODE {
	DMA_NOW = 0,
	DMA_AT_VBLANK,
	DMA_AT_HBLANK,
	DMA_AT_SPECIFIC
};
class Bus;

class Dma {
private:
	Bus* bus;
	enum DMA_NB dmaId;
public:
	u8 unusedValue; //Bits 0-4 of CONTROL
	u32 destAdress;
	u32 srcAdress;
	u16 countValue;
	u16 control;
	u8 destAdrControl;
	u8 srcAdrControl;
	bool repeat;
	enum DMA_TRANSFER_TYPE transferType;
	bool gamePakDrq; // dma 3 only
	enum DMA_TIMING_MODE timingMode;
	bool irqUponEnd;
	bool dmaEnable;

	static enum DMA_NB selectDma(u32 addr);
	void notify(u8 signal); // is called by the bus when a signal (HBLANK, VBLANK) is triggered
	void setBus(Bus* bus);
	void instantTransfer();
	void transfer();
	Dma(enum DMA_NB dmaId);
};


#endif // DMA_H