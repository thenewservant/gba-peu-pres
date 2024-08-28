#include "arm/arm7tdmi.h"
#include "ppu/ppu.h"
#include "bus/gba_bus.h"
#include "common/types.h"
#include "screen/screen.h"
#include <fstream>
#include <iostream>
#include <thread>

#define DISPLAY_MULTIPLIER 3

//#define TEST
void cpuRun(Arm7tdmi* cpu) {
	while (true) {
		cpu->tick();
	}
}

int main(int argc, char* argv[]) {
#ifdef WIN32
	SetProcessDPIAware();
#endif
	char* filename= 0;
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

	Screen* screen = new Screen(DISPLAY_MULTIPLIER, cpu);
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