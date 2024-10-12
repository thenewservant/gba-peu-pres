#ifndef PPU_H
#define PPU_H
#include "common/types.h"
#include "screen/screen.h"
#include "bus/gba_bus.h"

#define BG0_ON ((lcd.regs.dispcnt & 0x100) != 0)
#define BG1_ON ((lcd.regs.dispcnt & 0x200) != 0)
#define BG2_ON ((lcd.regs.dispcnt & 0x400) != 0)
#define BG3_ON ((lcd.regs.dispcnt & 0x800) != 0)

#define WIN0_ON ((lcd.regs.dispcnt & 0x1000) != 0)
#define WIN1_ON ((lcd.regs.dispcnt & 0x2000) != 0)

#define WINOBJ_ON ((lcd.regs.dispcnt & 0x4000) != 0)

enum TILE_BG_ID {
	BG0 = 0,
	BG1 = 1,
	BG2 = 2,
	BG3 = 3
};

PACK(struct lcdIORegs_t {
	u16 dispcnt;
	u16 padding1;
	u16 dispstat;
	u16 vcount;
	u16 bg0cnt;
	u16 bg1cnt;
	u16 bg2cnt;
	u16 bg3cnt;
	u16 bg0hofs;
	u16 bg0vofs;
	u16 bg1hofs;
	u16 bg1vofs;
	u16 bg2hofs;
	u16 bg2vofs;
	u16 bg3hofs;
	u16 bg3vofs;
	u16 bg2pa;
	u16 bg2pb;
	u16 bg2pc;
	u16 bg2pd;
	u32 bg2x;
	u32 bg2y;
	u16 bg3pa;
	u16 bg3pb;
	u16 bg3pc;
	u16 bg3pd;
	u32 bg3x;
	u32 bg3y;
	u16 win0h;
	u16 win1h;
	u16 win0v;
	u16 win1v;
	u16 winin;
	u16 winout;
	u16 mosaic;
	u16 padding2;
	u16 bldcnt;
	u16 bldalpha;
	u16 bldy;
	u16 padding3;
});

typedef union _lcdUnion_t {
	struct lcdIORegs_t regs; // lcd registers as a struct
	u8 array[sizeof(struct lcdIORegs_t)]; // lcd registers as an array
} LcdUnion;

class Ppu {
private:
	Bus* bus;
	Screen* screen;
	u32* pixels;
	u16 scanline = 0;
	u16 cycle = 0;
public:
	LcdUnion lcd; // union containing all the lcd registers
	Ppu(Screen* screen, Bus* bus);
	u8* readIO(u32 addr);
	void writeIO32(u32 addr, u32 data);
	void writeIO16(u32 addr, u16 data);
	void writeIO8(u32 addr, u8 data);
	void raiseHBlankIrqIfNeeded();
	void raiseVBlankIrqIfNeeded();
	void raiseVCountIrqIfNeeded();
	void updateDispstatAndVCount();
	void mode0SingleLineFeed(u32* line, enum TILE_BG_ID bgType, u16 scanline);
	void mode0Orchestrator(u32* line);
	void mode3();
	void mode4();
	void mode5();
	void tick();
	void obj();
};


#endif