#ifndef GBA_BUS_H
#define GBA_BUS_H

#include <cstdio>
#include <cstdlib>
#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

class Bus {
	private:
		u32 romSizeInBytes;
		/**** Main Mem ****/
		u8 bios[0x4000]; //00000000-00003FFF
		// 00004000-01FFFFFF is not used 
		u8 ewram[0x40000]; //02000000-0203FFFF ( 2 WAIT)
		// 02040000-02FFFFFF is not used
		u8 iwram[0x8000]; //03000000-03007FFF
		//03008000-03FFFFFF is not used
		// 04000000 - 040003FE -> IO registers
		// 04000400-04FFFFFF is not used

		/**** Display Mem ****/
		u8 palette_ram[0x400]; //05000000 - 050003FF Palette RAM 1KB
		// 05000400-05FFFFFF   Not used
		u8 vram[0x18000]; //06000000 - 06017FFF VRAM 96KB
		//06018000 - 06FFFFFF   Not used
		u8 oam[0x400]; //07000000 - 070003FF OAM 1KB
		// 07000400-07FFFFFF   Not used
		u8 rom[0x2000000]; //08000000 - 09FFFFFF ROM 32MB


	public:
		constexpr u8* getMemoryChunkFromAddress(u32 add);
		u8 read8(u32 addr);
		u16 read16(u32 addr);
		u32 read32(u32 addr);

		void write8(u32 addr, u8 data);
		void write16(u32 addr, u16 data);
		void write32(u32 addr, u32 data);

		void loadGamePack(const char* filename);

};



#endif // GBA_BUS_H