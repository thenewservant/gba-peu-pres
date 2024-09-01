#include "dma/dma_manager.h"
#include "dma/dma.h"
#include "bus/gba_bus.h"

DmaManager::DmaManager(Bus* bus) {
	for (int i = 0; i < 4; i++) {
		dmaArray[i] = new Dma((enum DMA_NB)i);
	}
}