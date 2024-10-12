#include "dma/dma.h"
#include "bus/gba_bus.h"

void Dma::notify(u8 signal) {
	// TODO
}

Dma::Dma(enum DMA_NB dmaId) {
	this->dmaId = dmaId;
}

enum DMA_NB Dma::selectDma(u32 addr) {
	return DMA0;
}

void Dma::setBus(Bus* bus) {
	this->bus = bus;
}

enum DMA_DST_CONTROL {
	DMA_DST_INCREMENT,
	DMA_DST_DECREMENT,
	DMA_DST_STATIC,
	DMA_DST_REFRESH
};

enum DMA_SRC_CONTROL {
	DMA_SRC_INCREMENT,
	DMA_SRC_DECREMENT,
	DMA_SRC_STATIC
};

void Dma::instantTransfer() {
	if (dmaEnable) {
		//printf("dma nb %d is transfering... ", dmaId);
		//printf("info: from @: %08x, to @: %08x, count: %x\n", srcAdress, destAdress, countValue);
		u16 unitsLeft = countValue;
		u32 src = srcAdress;
		u32 dest = destAdress;
		u8 wordSize = (transferType == DMA_16BIT) ? 2 : 4;
		u8 srcIncrement=0, destIncrement=0;
		switch (srcAdrControl) {
		case DMA_SRC_INCREMENT:
			srcIncrement = wordSize;
			break;
		case DMA_SRC_DECREMENT:
			srcIncrement = -wordSize;
			break;
		case DMA_SRC_STATIC:
			srcIncrement = 0;
			break;
		}

		switch (destAdrControl) {
		case DMA_DST_INCREMENT:
			destIncrement = wordSize;
			break;
		case DMA_DST_DECREMENT:
			destIncrement = -wordSize;
			break;
		case DMA_DST_STATIC:
			destIncrement = 0;
			break;
		case DMA_DST_REFRESH:
			destIncrement = wordSize;
			//TODO: reset it so that the refresh takes place back at the beginning
			break;
		}
		if (wordSize == 2) {
			for (int i = 0; i < unitsLeft; i++) {
				u16 data = bus->read16(src);
				bus->write16(dest, data);
				src += srcIncrement;
				dest += destIncrement;
			}
		}
		else {
			for (int i = 0; i < unitsLeft; i++) {
				u32 data = bus->read32(src);
				bus->write32(dest, data);
				src += srcIncrement;
				dest += destIncrement;
			}
		}
	}
	dmaEnable = false;
	if (irqUponEnd) {
		this->bus->intCtrl.regs.if_ |= 1 << (dmaId + 8);
	}
}

void Dma::transfer() {
	// time taken: 2N+2(n-1)S+xI
	// Internal time for DMA processing is 2I (normally), or 4I (if both source and destination are in gamepak memory area).
	//1N+(n-1)S are read cycles, and the other 1N+(n-1)S are write cycles, actual number of cycles depends on the waitstates and bus-width of the source and destination areas

	instantTransfer();
}