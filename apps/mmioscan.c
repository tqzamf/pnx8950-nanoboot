#include <stdint.h>
#include <hw.h>

#include "lib.c"

#define MMIO_BASE 0xbbe00000
#define MMIO_SIZE 2*1024*1024

int main(void) {
	// flush previous output, especially xmodem
	printf("\nMMIOscan\n");
	
	unsigned int *page = (unsigned int *) MMIO_BASE;
	for (int base = 0; base < (MMIO_SIZE >> 2); base += 4096/4) {
		unsigned int data = page[base + 4096/4 - 1];
		unsigned int start = (base << 2);

		printf("%08x-", start);
		if (data == 0xdeadabba || data == 0x00000000) { // dead bus
			printf("%08x: %08x   -- skipped --\n", start + 0xfff, data);
			continue;
		}
		
		unsigned int module = data >> 16;
		unsigned int major = (data >> 12) & 15;
		unsigned int minor = (data >> 8) & 15;
		unsigned int aperture = (data & 255);
		unsigned int end = (base << 2) + 4096 * aperture + 0xfff;
		
		printf("%08x: %08x ", end, data);
		if (module == 0x0124) // null module / hole
			printf("-- null -- ");
		else
			printf("module %04x", module);
		printf(" v%d.%d, %dk\n", major, minor, 4 * (aperture + 1));
		
		// skip to next module
		base += 4096/4 * aperture;
	}
	
	return 0;
}
