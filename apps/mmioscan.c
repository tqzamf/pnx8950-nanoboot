#include <stdint.h>
#include <hw.h>

#include "lib.c"

#define MMIO_BASE 0xbbe00000
#define MMIO_SIZE 2*1024*1024

int main(void) {
	// flush previous output, especially xmodem
	puts("\r\n\r\nMMIOscan\r\n");
	
	unsigned int *page = (unsigned int *) MMIO_BASE;
	for (int base = 0; base < (MMIO_SIZE >> 2); base += 4096/4) {
		unsigned int data = page[base + 4096/4 - 1];
		unsigned int start = (base << 2);

		putx32(start);
		puts("-");
		if (data == 0xdeadabba || data == 0x00000000) { // dead bus
			putx32(start + 0xfff);
			puts(": ");
			putx32(data);
			puts("   -- skipped --\r\n");
			continue;
		}
		
		unsigned int module = data >> 16;
		unsigned int major = (data >> 12) & 15;
		unsigned int minor = (data >> 8) & 15;
		unsigned int aperture = (data & 255);
		unsigned int end = (base << 2) + 4096 * aperture + 0xfff;
		
		putx32(end);
		puts(": ");
		putx32(data);
		if (module == 0x0124) { // null module / hole
			puts(" no module  ");
		} else {
			puts(" module ");
			putx16(module);
		}
		puts(" v");
		putd(major);
		puts(".");
		putd(minor);
		puts(", ");
		putd(4 * (aperture + 1));
		puts("k\r\n");
		
		// skip to next module
		base += 4096/4 * aperture;
	}
	
	return 0;
}
