#include "arm/arm7tdmi.h"
#include "ppu/ppu.h"
#include "arm/tests/tests.h"
#include "bus/gba_bus.h"
#include "common/types.h"
#include "screen/screen.h"
#include <fstream>
#include <iostream>
#include <string>

#include <thread>

void cpuRun(Arm7tdmi* cpu) {
	static int i = 0;
	
	while (true) {
		printf("instruction sprint count (1000): %d\n", i++);
		for (int i = 0; i < 1000; i++) {
			cpu->tick();
		}
	}
}

int main(int argc, char* argv[]) {
	
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

	Arm7tdmi* cpu = new Arm7tdmi(bus);

	Screen* screen = new Screen(3, cpu);
	Ppu* ppu = new Ppu(screen, bus);
	ppu->ppuRegs[0] = 0x91;
	cpu->setPPU(ppu);
	bus->setPPU(ppu);
	

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
	return 0;
}