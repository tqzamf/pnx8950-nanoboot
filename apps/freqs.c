#include <stdint.h>
#include <hw.h>

#include "lib.c"

#define MMIO_CLOCK_BASE 0xbbe47000
#define MMIO_CLOCK_SIZE 4096

int main(void) {
	// flush previous output, especially xmodem
	printf("\nclock module frequencies\n");
	
	printf("   ");
	#define HEADER(x) \
		printf("    %02x", x);
	#define HEADER2(x) \
		HEADER(x+0); \
		HEADER(x+1); \
		HEADER(x+2); \
		HEADER(x+3);
	HEADER2(0);
	HEADER2(4);
	printf(" ");
	HEADER2(8);
	HEADER2(12);
	printf("\n");

	volatile unsigned int *cm = (unsigned int *) MMIO_CLOCK_BASE;
	volatile unsigned int *fcr = &cm[0x104 >> 2];
	for (int sel = 0; sel < 128; sel++) {
		if ((sel & 15) == 0)
			printf("%02x ", sel);

		*fcr = (sel << 4) | 1;
		while (!(*fcr & 2));
		unsigned short res = *fcr >> 16;
		
		int integer = res / 10;
		int fractional = res % 10;
		printf(" %3d.%d", integer, fractional);
		if ((sel & 15) == 7)
			printf(" ");
		if ((sel & 15) == 15)
			printf("\n");
	}

	return 0;
}
