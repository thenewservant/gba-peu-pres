#include "ppu.h"

#define MAX_OBJECTS 128

#define OAM_BASE 0x07000000

#define OBJ_Y_COORDINATE_MASK 0x0FF // bits 0-7. OAM attribute 0
#define OBJ_ROTATION_ON_FLAG 0x100 // bit 8. OAM attribute 0
// bit 9. OAM attribute 0.
// If OBJ_ROTATION_ON_FLAG is SET, then OBJ_SIZE_FLAG is used to determine the size of the object
// (Normal or double). Else, SET equals OBJ shown, otherwise hidden.
#define OBJ_MODE_FLAG 0x200 

#define OBJ_IS_SHOWN(at0) ((at0 & (OBJ_ROTATION_ON_FLAG | OBJ_ROTATION_ON_FLAG)) == 0)

#define OBJ_X_COORDINATE_MASK 0x1FF // bits 0-8. OAM attribute 1

void Ppu::obj() {//pixel based strategy for now.
	u8 totalObjFound = 0;
	for (u8 objIdx = 0; objIdx < MAX_OBJECTS; objIdx++) {
		
		u16 objAttr0 = bus->read16OAM(objIdx * 8);
		u16 objAttr1 = bus->read16OAM(objIdx * 8 + 2);
		u16 xPos = objAttr1 & OBJ_X_COORDINATE_MASK;
		u16 yPos = objAttr0 & OBJ_Y_COORDINATE_MASK;
		if (OBJ_IS_SHOWN(objAttr0)) {
			totalObjFound++;
			if (yPos == scanline && xPos == cycle) {
				screen->getPixels()[scanline * SCREEN_WIDTH + cycle] = 0x00FF00; //green
				
			}
		}
	}
	
}