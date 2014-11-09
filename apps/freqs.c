#include <stdint.h>
#include <hw.h>

#include "lib.c"

#define MMIO_CLOCK_BASE 0xbbe47000
#define MMIO_CLOCK_SIZE 4096

int main(void) {
	// flush previous output, especially xmodem
	puts("\r\n\r\nclock module frequencies\r\n");
	
	puts("   ");
	#define HEADER(x) \
		puts("    "); \
		putx8(x);
	#define HEADER2(x) \
		HEADER(x+0); \
		HEADER(x+1); \
		HEADER(x+2); \
		HEADER(x+3);
	HEADER2(0);
	HEADER2(4);
	puts(" ");
	HEADER2(8);
	HEADER2(12);
	puts("\r\n");

	volatile unsigned int *cm = (unsigned int *) MMIO_CLOCK_BASE;
	volatile unsigned int *fcr = &cm[0x104 >> 2];
	for (int sel = 0; sel < 128; sel++) {
		if ((sel & 15) == 0) {
			putx8(sel);
			puts(" ");
		}

		*fcr = (sel << 4) | 1;
		while (!(*fcr & 2));
		unsigned short res = *fcr >> 16;
		
		int integer = res / 10;
		int fractional = res % 10;
		puts(" ");
		if (integer < 10)
			puts("  ");
		else if (integer < 100)
			puts(" ");
		putd(integer);
		puts(".");
		putd(fractional);
		if ((sel & 15) == 7)
			puts(" ");
		if ((sel & 15) == 15)
			puts("\r\n");
	}

	return 0;
}
