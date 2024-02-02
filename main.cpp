#include "arm/arm7tdmi.h"
#include "arm/tests/tests.h"
#include "bus/gba_bus.h"
#include "common/types.h"
#include "screen/screen.h"
#include <fstream>
#include <iostream>
#include <string>

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
	Screen* screen = new Screen(1, cpu);

	while(true){
		screen->listener();
		//SDL_Delay(1);
	}
	return 0;
}