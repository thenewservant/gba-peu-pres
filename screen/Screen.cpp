#include "Screen.h"


void Screen::initSDLScreen() {
	screenW = SCREEN_WIDTH * ScreenScaleFactor;
	screenH = SCREEN_HEIGHT * ScreenScaleFactor;
	window = SDL_CreateWindow("gbapeupres", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenW, screenH, SDL_WINDOW_SHOWN | 0 * SDL_WINDOW_RESIZABLE);
	
	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | 1*SDL_RENDERER_PRESENTVSYNC);
	
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
	keysStatus = 0xFFFF;
	initSDLScreen();
}

void Screen::advance() {
	cpu->tick();
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
	case SDLK_i:
		cpu->printRegsUserMode();
		break;
	case SDLK_RETURN:
		keysStatus &= ~8;
		break;
	case SDLK_DOWN:
		keysStatus &= ~0x80;
		break;
	case SDLK_UP:
		keysStatus &= ~0x40;
		break;
	case SDLK_LEFT:
		keysStatus &= ~0x20;
		break;
	case SDLK_RIGHT:
		keysStatus &= ~0x10;
		break;
	case SDLK_a://L1 key
		keysStatus &= ~0x200;
		break;
	case SDLK_e://R1 key
		keysStatus &= ~0x100;
		break;
	case SDLK_k:
		keysStatus &= ~0x2;
		break;
	case SDLK_l:
		keysStatus &= ~0x1;
		break;
	case SDLK_RSHIFT://select
		keysStatus &= ~0x4;
		break;
	case SDLK_F9:
		advance();
		cpu->printRegsUserMode();
		break;
	case SDLK_F10:
		printf("100 ticks at a time\n");
		for (int i = 0; i < 100; i++){
			advance();
			cpu->printRegsUserMode();
		}
		
		break;
	case SDLK_F11:
		printf("10000 ticks at a time\n");
		for (int i = 0; i < 10000; i++) {
			advance();
		}

		break;
	default:
		break;
	}
}

void Screen::checkRaiseKey( SDL_Event event) {
	switch (event.key.keysym.sym) {

	case SDLK_RETURN:
		keysStatus |= 8;
		break;
	case SDLK_DOWN:
		keysStatus |= 0x80;
		break;
	case SDLK_UP:
		keysStatus |= 0x40;
		break;
	case SDLK_LEFT:
		keysStatus |= 0x20;
		break;
	case SDLK_RIGHT:
		keysStatus |= 0x10;
		break;
	case SDLK_a://L1 key
		keysStatus |= 0x200;
		break;
	case SDLK_e:
		keysStatus |= 0x100;
		break;
	case SDLK_l://A key
		keysStatus |= 0x1;
		break;
	case SDLK_k://B key
		keysStatus |= 0x2;
		break;
	case SDLK_RSHIFT://select
		keysStatus |= 0x4;
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
			cpu->bus->setKeysStatus(keysStatus);
			break;
		case SDL_KEYUP:
			checkRaiseKey(keyEvent);
			cpu->bus->setKeysStatus(keysStatus);
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
				break;
			default:
				break;
			}
		default:
			break;
		}
	}

	SDL_Delay(2);
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
