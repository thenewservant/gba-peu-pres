#include "Screen.h"


void Screen::initSDLScreen() {
	screenW = SCREEN_WIDTH * ScreenScaleFactor;
	screenH = SCREEN_HEIGHT * ScreenScaleFactor;
	window = SDL_CreateWindow("gbapeupres", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenW, screenH, SDL_WINDOW_SHOWN | 0 * SDL_WINDOW_RESIZABLE);
	
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	// Create SDL texture
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(1);
	}
}

void Screen::endSDLApplication() {
	// Cleanup SDL
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

Screen::Screen(u8 scaleFact) {
	status = 0;
	ScreenScaleFactor = scaleFact;
	initSDLScreen();
}

Screen::Screen(u8 scaleFact, Arm7tdmi* cpu) {
	this->cpu = cpu;
	status = 0;
	ScreenScaleFactor = scaleFact;
	initSDLScreen();
}

void Screen::advance() {
	cpu->tick();
	cpu->printRegsUserMode();
}

u32* Screen::getPixels() {
	return pixels;
}

void Screen::updateScreen() {
	SDL_RenderClear(renderer);//is it useful at all?
	SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(Uint32));
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void Screen::checkPressKey(SDL_Event event) {
	switch (event.key.keysym.sym) {
	case SDLK_y:
		status |= 0b100000000;
	case SDLK_F10:
		advance();
		break;
	case SDLK_F11:
		printf("100 ticks at a time\n");
		for (int i = 0; i < 100; i++){
			advance();
		}
		
		break;
	default:
		break;
	}
}

void Screen::checkRaiseKey( SDL_Event event) {
	switch (event.key.keysym.sym) {

	case SDLK_y:
		status &= ~0b100000000;
		break;
	default:
		break;
	}
}

u8 Screen::listener() {

	while (SDL_PollEvent(&keyEvent)) {
		switch (keyEvent.type) {
		case SDL_KEYDOWN:
			checkPressKey(keyEvent);
			break;
		case SDL_KEYUP:
			checkRaiseKey(keyEvent);
			break;
		case SDL_DROPFILE:
			printf("File dropped: %s\n", keyEvent.drop.file);
			newFilePath = keyEvent.drop.file;
			return 10;
			break;
		case SDL_WINDOWEVENT:
			switch (keyEvent.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				printf("Window closed\n");
				exit(0);
			case SDL_WINDOWEVENT_RESIZED:
				printf("Window resized\n");
				//resizeWindow();
				break;
			default:
				break;
			}
		default:
			break;
		}
	}

	SDL_Delay(1);
	return 0;
}

SDL_Window* Screen::getWindow() {
	return window;
}

void Screen::writePixel(u32 where, u32 what) {
	pixels[where] = what;
}

char* Screen::getFilePath() {
	return newFilePath;
}
