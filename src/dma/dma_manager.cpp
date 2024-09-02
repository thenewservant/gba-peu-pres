#include "dma/dma_manager.h"
#include "dma/dma.h"
#include "bus/gba_bus.h"

Dma DmaManager::dmas[4] = { Dma(DMA_NB::DMA0), Dma(DMA_NB::DMA1), Dma(DMA_NB::DMA2), Dma(DMA_NB::DMA3)};

DmaManager::DmaManager(Bus* bus) {

}

void DmaManager::write8(u32 addr, u8 data){
	u8 dmaId = (addr - DMA_FIRST_MAP_ADDRESS) / DMA_SPACE_SIZE;

}

void DmaManager::write16(u32 addr, u16 data){

}

void DmaManager::write32(u32 addr, u32 data) {

}

u32 DmaManager::read32(u32 addr)
{
	return u32();
}

u16 DmaManager::read16(u32 addr)
{
	return u16();
}

u8 DmaManager::read8(u32 addr)
{
	return u8();
}
