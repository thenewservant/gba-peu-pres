#include "dma.h"
#include "../bus/gba_bus.h"

void notify(u8 signal) {
	// TODO
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