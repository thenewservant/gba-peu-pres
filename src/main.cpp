#include "arm/arm7tdmi.h"
#include "ppu/ppu.h"
#include "bus/gba_bus.h"
#include "common/types.h"
#include "screen/screen.h"
#include "tests.h"
#include <thread>
#include <windows.h>

#define DISPLAY_MULTIPLIER 3

//#define TEST
void cpuRun(Arm7tdmi* cpu) {
	while (true) {
		cpu->tick();
	}
}
static const char* getFileNameFromPath(const char* path);

int main(int argc, char* argv[]) {
#ifdef WIN32
	SetProcessDPIAware();
#endif
	char* filename = 0;
	if (argc > 1) {
		filename = argv[1];
	}
	else {
		printf("Usage: %s <gamepack>\n", argv[0]);
		exit(1);
	}
	Bus* bus = new Bus();
	bus->loadGamePack(filename);
	if (argc > 2) {
		bus->loadBios(argv[2]);
	}
	Arm7tdmi* cpu = new Arm7tdmi(bus);

	Screen* screen = new Screen(DISPLAY_MULTIPLIER, cpu, getFileNameFromPath(argv[1]));
	Ppu* ppu = new Ppu(screen, bus);
	cpu->setPPU(ppu);
	bus->setPPU(ppu);
#ifdef TEST
	//test_LDM_IA_DB(new Arm7tdmi(new Bus()));
	testSequence1(new Arm7tdmi(new Bus()));
	testWriteToEWRAM(new Arm7tdmi(new Bus()));
#else 
#ifndef DEBUG
	//thread for cpuRun
	std::thread cpuThread(cpuRun, cpu);
	while (true) {
		screen->listener();
		//SDL_Delay(1);
	}
	cpuThread.join();
#else
	while (true) {
		screen->listener();
		//SDL_Delay(1);
	}
#endif
#endif
	return 0;
}

static const char* getFileNameFromPath(const char* path) {
	if (path == nullptr) {
		return nullptr;
	}

	const char* fileName = path;
	const char* slash = strrchr(path, '/');
	const char* backslash = strrchr(path, '\\');

	if (slash || backslash) {
		// Use the last slash or backslash found, whichever comes last.
		const char* separator = (slash > backslash) ? slash : backslash;
		fileName = separator + 1;
	}

	char* fn2 = (char*)malloc(100 * sizeof(char));
	if (fn2 == nullptr) {
		return nullptr;
	}

	strcpy(fn2, fileName);
	char* dot = strrchr(fn2, '.');
	if (dot) {
		*dot = '\0';
	}
	return fn2;
}