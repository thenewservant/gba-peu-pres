#ifndef SCREEN_H
#define SCREEN_H
#include <SDL.h>
#include "common/types.h"
#include "arm/arm7tdmi.h"
#include "bus/gba_bus.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

#define FILE_DROPPED 184

class Screen {
private:
	u16 keysStatus;
	Arm7tdmi* cpu;
	char title[150];
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	u8 ScreenScaleFactor;
	u16 status;
	bool needFullScreenToggle = false;
	SDL_Event keyEvent;
	u16 screenW, screenH;
	u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
private:
	void initSDLScreen();
	char* newFilePath;
public:
	void advance();//tick cpu and update screen
	Screen(u8 scaleFact);
	Screen(u8 scaleFact, Arm7tdmi* cpu);
	u32* getPixels();
	void updateScreen();
	void checkPressKey(SDL_Event event);
	void checkRaiseKey(SDL_Event event);
	void endSDLApplication();
	u8 listener();
	SDL_Window* getWindow();
	void writePixel(u32 where, u32 what);
	char* getFilePath();
	
};

#endif