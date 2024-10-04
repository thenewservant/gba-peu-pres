#include "dma/dma_manager.h"
#include "dma/dma.h"
#include "bus/gba_bus.h"

Dma DmaManager::dmas[4] = { Dma(DMA_NB::DMA0), Dma(DMA_NB::DMA1), Dma(DMA_NB::DMA2), Dma(DMA_NB::DMA3) };

DmaManager::DmaManager(Bus* bus) {
	for (int i = 0; i < 4; i++) {
		dmas[i].setBus(bus);
	}
}

void DmaManager::tick(){
	for (int i = 0; i < 4; i++) {
		dmas[i].transfer();
	}
}

void DmaManager::writeControl0(u8 dmaId, u8 data) {
	dmas[dmaId].unusedValue = data & 0x1F;
	dmas[dmaId].destAdrControl = (data >> 5) & 0x3;
	dmas[dmaId].srcAdrControl = dmas[dmaId].srcAdrControl & 0b10 | (data >> 7) & 0x1;
}

void DmaManager::writeControl1(u8 dmaId, u8 data) {
	dmas[dmaId].srcAdrControl = dmas[dmaId].srcAdrControl & 0b01 | (data) & 0x1;
	dmas[dmaId].transferType =(enum DMA_TRANSFER_TYPE)( (data >> 2) & 0x1);
	dmas[dmaId].timingMode = (enum DMA_TIMING_MODE)( (data >> 4) & 0x3);
	dmas[dmaId].irqUponEnd = (bool)((data >> 6) & 0x1);
	dmas[dmaId].dmaEnable = (bool)((data >> 7) & 0x1);
}

void DmaManager::write8(u32 addr, u8 data) {
	u8 dmaId = (addr - DMA_FIRST_MAP_ADDRESS) / DMA_SPACE_SIZE;
	enum DMA_FIELDS field = (enum DMA_FIELDS)((addr - DMA_FIRST_MAP_ADDRESS) % DMA_SPACE_SIZE);
	switch (field) {
	case DMA_SOURCE_ADRESS_0:
		dmas[dmaId].srcAdress = (dmas[dmaId].srcAdress & 0xFFFFFF00) | data;
		break;
	case DMA_SOURCE_ADRESS_1:
		dmas[dmaId].srcAdress = (dmas[dmaId].srcAdress & 0xFFFF00FF) | (data << 8);
		break;
	case DMA_SOURCE_ADRESS_2:
		dmas[dmaId].srcAdress = (dmas[dmaId].srcAdress & 0xFF00FFFF) | (data << 16);
		break;
	case DMA_SOURCE_ADRESS_3:
		dmas[dmaId].srcAdress = (dmas[dmaId].srcAdress & 0x00FFFFFF) | (data << 24);
		break;
	case DMA_DESTINATION_ADRESS_0:
		dmas[dmaId].destAdress = (dmas[dmaId].destAdress & 0xFFFFFF00) | data;
		break;
	case DMA_DESTINATION_ADRESS_1:
		dmas[dmaId].destAdress = (dmas[dmaId].destAdress & 0xFFFF00FF) | (data << 8);
		break;
	case DMA_DESTINATION_ADRESS_2:
		dmas[dmaId].destAdress = (dmas[dmaId].destAdress & 0xFF00FFFF) | (data << 16);
		break;
	case DMA_DESTINATION_ADRESS_3:
		dmas[dmaId].destAdress = (dmas[dmaId].destAdress & 0x00FFFFFF) | (data << 24);
		break;
	case DMA_WORD_COUNT_0:
		dmas[dmaId].countValue = (dmas[dmaId].countValue & 0xFF00) | data;
		break;
	case DMA_WORD_COUNT_1:
		dmas[dmaId].countValue = (dmas[dmaId].countValue & 0x00FF) | (data << 8);
		break;
	case DMA_CONTROL_0:
		writeControl0(dmaId, data);
		break;
	case DMA_CONTROL_1:
		writeControl1(dmaId, data);
		break;
	default:
		break;
	}
}

void DmaManager::write16(u32 addr, u16 data) {
	u8 firstByte = data & 0xFF;
	u8 secondByte = (data >> 8) & 0xFF;
	write8(addr, firstByte);
	write8(addr + 1, secondByte);
}

void DmaManager::write32(u32 addr, u32 data) {
	u16 firstHalf = data & 0xFFFF;
	u16 secondHalf = (data >> 16) & 0xFFFF;
	write16(addr, firstHalf);
	write16(addr + 2, secondHalf);
}

u8 DmaManager::readControl0(u8 dmaId) {
	u8 control0 = dmas[dmaId].unusedValue;
	control0 |= dmas[dmaId].destAdrControl << 5;
	control0 |= dmas[dmaId].srcAdrControl << 7;
	return control0;
}

u8 DmaManager::readControl1(u8 dmaId) {
	u8 control1 = dmas[dmaId].srcAdrControl & 0b01;
	control1 |= dmas[dmaId].transferType << 10;
	control1 |= dmas[dmaId].timingMode << 12;
	control1 |= dmas[dmaId].irqUponEnd << 14;
	control1 |= dmas[dmaId].dmaEnable << 15;
	return control1;
}

u8 DmaManager::read8(u32 addr)
{
	u8 dmaId = (addr - DMA_FIRST_MAP_ADDRESS) / DMA_SPACE_SIZE;
	enum DMA_FIELDS field = (enum DMA_FIELDS)((addr - DMA_FIRST_MAP_ADDRESS) % DMA_SPACE_SIZE);
	switch (field) {
	case DMA_CONTROL_0:
		return readControl0(dmaId);
	case DMA_CONTROL_1:
		return readControl1(dmaId);
	case DMA_SOURCE_ADRESS_0:
	case DMA_SOURCE_ADRESS_1:
	case DMA_SOURCE_ADRESS_2:
	case DMA_SOURCE_ADRESS_3:
	case DMA_DESTINATION_ADRESS_0:
	case DMA_DESTINATION_ADRESS_1:
	case DMA_DESTINATION_ADRESS_2:
	case DMA_DESTINATION_ADRESS_3:
	case DMA_WORD_COUNT_0:
	case DMA_WORD_COUNT_1:
	default:
		printf("WARNING: unable to READ from DMA field @ %08X\n", addr);
		return 0;
	}
}

u16 DmaManager::read16(u32 addr)
{
	u8 firstByte = read8(addr);
	u8 secondByte = read8(addr + 1);
	return (secondByte << 8) | firstByte;
}

u32 DmaManager::read32(u32 addr)
{
	u16 firstHalf = read16(addr);
	u16 secondHalf = read16(addr + 2);
	return (secondHalf << 16) | firstHalf;

}