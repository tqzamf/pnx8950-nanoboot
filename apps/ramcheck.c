#include <stdint.h>
#include <hw.h>

#include "lib.c"

int main(void) {
	// flush previous output, especially xmodem
	printf("\nRAMcheck\n");
	
	// initialize RC4 state
	uint8_t s[256];
	for (int i = 0; i < 256; i++)
		s[i] = i;
	uint8_t j = 0;
	for (int i = 0; i < 256; i++) {
		j += s[i] + "\xde\xad\xab\xba"[i&3];
		uint8_t temp = s[i];
		s[i] = s[j];
		s[j] = temp;
	}
	j = 0;
	uint8_t i = 0;
	
	int global_err = 0;
	// check every MB separately
	for (uint32_t addr = 0x80000000; addr < 0x90000000; addr += 0x100000) {
		printf("%08x: ", addr);
		if (addr == 0x84000000 /* nanoboot */
				|| addr == 0x80100000 /* ramcheck itself */
				|| addr == 0x87e00000 /* U-Boot's PCI DMA area */
			) {
			// that's us; don't shoot ourselves!
			printf("skipped\r\n");
			continue;
		}
		
		// preserve RC4 state for checking
		uint8_t t[256];
		for (int i = 0; i < 256; i++)
			t[i] = s[i];
		uint8_t ii = i, jj = j;

		// fill block according to RC4 output
		volatile uint8_t *block = (uint8_t *) addr;
		for (int k = 0; k < 0x100000; k++) {
			i++;
			j += s[i];
			uint8_t temp = s[i];
			s[i] = s[j];
			s[j] = temp;
			block[k] = s[(s[i] + s[j]) & 255];
		}
		
		// verify block against recomputed RC4 output
		int err = 0;
		for (int k = 0; k < 0x100000; k++) {
			ii++;
			jj += t[ii];
			uint8_t temp = t[ii];
			t[ii] = t[jj];
			t[jj] = temp;
			uint8_t x = t[(t[ii] + t[jj]) & 255];
			
			if (block[k] != x) {
				// block mismatch, probably bad RAM: warn
				printf("\n\t!%08x %02x %02x", (uint32_t) &block[k],
						block[k], x);
				err = 1;
				global_err = 1;
			}
		}
		
		// status
		if (!err)
			printf("OK");
		printf("\n");
	}
	if (global_err)
		printf("WARNING: errors detected! check log for details\n");
	else
		printf("all memory OK\n");
	// return to nanoboot. for U-Boot, this crashes the system, because
	// U-Boot has been overwritten by ramcheck.
	return 0;
}
