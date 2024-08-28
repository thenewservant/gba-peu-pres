#include "dma/dma.h"
#include "bus/gba_bus.h"

void Dma::notify(u8 signal) {
	// TODO
}

Dma::Dma(enum DMA_NB dmaId) {
	this->dmaId = dmaId;
}

enum DMA_NB Dma::selectDma(u32 addr) {
	if (addr < 0x040000BC) {
		return DMA0;
	}
	else if (addr < 0x040000C8) {
		return DMA1;
	}
	else if (addr < 0x040000D4) {
		return DMA2;
	}
	else if (addr < 0x040000E0) {
		return DMA3;
	}
}

u8* Dma::readIO(u32 add) {
	return controlRegs.array + (add & 0x000000FF);
}

void Dma::setBus(Bus* bus) {
	this->bus = bus;
}

void Dma::transfer() {
	// time taken: 2N+2(n-1)S+xI
	// Internal time for DMA processing is 2I (normally), or 4I (if both source and destination are in gamepak memory area).
	//1N+(n-1)S are read cycles, and the other 1N+(n-1)S are write cycles, actual number of cycles depends on the waitstates and bus-width of the source and destination areas


}