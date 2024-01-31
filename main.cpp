#include "arm/arm7tdmi.h"
#include "arm/tests/tests.h"
#include "bus/gba_bus.h"
#include "common/types.h"
#include <fstream>
#include <iostream>
#include <string>

long long getFileSize(const char* s);

int main(int argc, char* argv[]) {
	Bus* bus = new Bus();
	Arm7tdmi* cpu = new Arm7tdmi(bus);

	while (0) {
		if (cpu->cpsr & T) { // Thumb mode
			u16 op = 0;// cpu.read16(r[15]);
			cpu->evaluateThumb(op);
			cpu->r[15] += 2;
		}
		else { // ARM mode
			u32 op = 0;// cpu.read32(r[15]);
			cpu->evaluateArm(op);

			cpu->r[15] += 4;
		}
	}
	if (argc > 1) {
		printf("%s\n", argv[1]);
		printf("%lld\n", getFileSize(argv[1]));
	}
	
	testAddVFLAG(cpu);
	test_ADD();
	test_SUB();
	test_SUB2();
	test_CMP();
	cpu->printRegsUserMode();
}

using namespace std;

long long getFileSize(const char* s)
{
	std::streampos fsize = 0;

	std::ifstream myfile(s, ios::in);  // File is of type const char*

	fsize = myfile.tellg();         // The file pointer is currently at the beginning
	myfile.seekg(0, ios::end);      // Place the file pointer at the end of file

	fsize = myfile.tellg() - fsize;
	myfile.close();

	static_assert(sizeof(fsize) >= sizeof(long long), "Oops.");

	std::cout << "size is: " << fsize << " bytes.\n";
	return fsize;
}