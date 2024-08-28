#ifndef GBA_BUS_H
#define GBA_BUS_H

#include "../dma/dma.h"
#include "../common/types.h"
#include "../timer/timer_manager.h"

u32 vramMirroredAdress(u32 addr);

PACK(struct intCtrlRegs_t {
	u16 ie; // Interrupt enable register
	u16 if_; // Interrupt request flags / IRQ Acknowledge
	u16 waitcnt; // Waitstate control
	u16 padding1;
	u16 ime; // Interrupt Master Enable
	u16 padding2;
});

typedef union _intCtrlUnion_t {
	struct intCtrlRegs_t regs;
	u8 array[sizeof(regs)]; // Interrupt control registers as an array
}InterruptControlUnion;

typedef union _pwrStatusUnion_t {
	PACK(struct regs {
		u8 postflg;
		u8 haltcnt;
	});
	u8 array[sizeof(regs)];
}PwrStatusUnion;

class Ppu;
class Arm7tdmi;

class Bus {
private:
	Dma dmaArray[4];
	Arm7tdmi* cpu;
	Ppu* ppu;
	
	PwrStatusUnion pwrStatus;
private:
	u32 romSizeInBytes;
	/**** Main Mem ****/
	u8 bios[0x4000]; //00000000-00003FFF
	// 00004000-01FFFFFF is not used 
	u8 ewram[0x40000]; //02000000-0203FFFF ( 2 WAIT)  -On Board Work RAM 256KB
	// 02040000-02FFFFFF is not used
	u8 iwram[0x8000]; //03000000-03007FFF  - Internal Work RAM 32KB
	//03008000-03FFFFFF is not used
	// 04000000 - 040003FE -> IO registers
	// 04000400-04FFFFFF is not used

	/**** Display Mem ****/
	u8 palette_ram[0x400]; //05000000 - 050003FF Palette RAM 1KB
	// 05000400-05FFFFFF   Not used
	u8 vram[0x20000]; //06000000 - 06017FFF VRAM 96KB
	//06018000 - 06FFFFFF   Not used
	u8 oam[0x400]; //07000000 - 070003FF OAM 1KB
	// 07000400-07FFFFFF   Not used
	u8 rom[0x2000000]; //08000000 - 09FFFFFF ROM 32MB

	u8 sram[0x10000]; //0E000000 - 0E00FFFF GamePak SRAM 64KB
	u32 potHole; // if invalid write is attempted (wrong width), write there instead
	u16 keysStatus;
	u32 internalMemoryControl; // at 0x4000804 (undocumented)
private:
	inline u8* ioAccess(u32 add);
public:
	TimerManager* timerManager;
	InterruptControlUnion intCtrl;
public:
	void setPPU(Ppu* p) {
		ppu = p;
	};
	
	u8 read8(u32 addr);
	u16 read16(u32 addr);
	u32 read32(u32 addr);

	u8 read8VRAM(u32 addr);
	u16 read16Palette(u32 addr);
	//Mainly for PPU reads
	u16 read16OAM(u32 addr);

	void write8(u32 addr, u8 data);
	void write16(u32 addr, u16 data);
	void write32(u32 addr, u32 data);

	void loadGamePack(const char* filename);
	void loadBios(const char* filename);
	void setKeysStatus(u16 keys) {
		keysStatus = keys;
	}
	Bus();
	u8* writeIo(u32 adress, u32 data);
	void ioWrite32(u32 addr, u32 data);
	void ioWrite16(u32 addr, u16 data);
	u32 ioRead32(u32 addr);
	u16 ioRead16(u32 addr);
	u8 ioRead8(u32 addr);
};

#endif // GBA_BUS_H