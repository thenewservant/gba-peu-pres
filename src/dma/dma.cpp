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

void Dma::instantTransfer() {
	if (dmaEnable) {
		printf("dma nb %d is transfering\n", dmaId);
		u16 byteCountLeft = countValue;
		u32 src = srcAdress;
		u32 dest = destAdress;
		u8 wordSize = transferType == DMA_16BIT ? 2 : 4;
		u8 srcIncrement = srcAdrControl & 0b01 ? -wordSize : wordSize;
		u8 destIncrement = destAdrControl & 0b01 ? -wordSize : wordSize;
		if (wordSize == 2) {
			for (int i = 0; i < byteCountLeft; i += wordSize) {
				u16 data = bus->read16(src);
				bus->write16(dest, data);
				src += srcIncrement;
				dest += destIncrement;
			}
		}
		else {
			for (int i = 0; i < byteCountLeft; i += wordSize) {
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