#include <stdint.h>
#include <hw.h>

int puts(char *string) {
	while (*string) {
		UART_TX((uint8_t) *string);
		string++;
		UART_FLUSH();
	}
	return 0;
}

void putx(uint32_t val) {
	for (int i = 0; i < 8; i++) {
		uint32_t nybble = val >> 28;
		if (nybble <= 9)
			UART_TX(nybble + '0');
		else
			UART_TX(nybble + 'a' - 10);
		val <<= 4;
	}
	UART_FLUSH();
}

void put8(uint8_t val) {
	uint32_t nybble = val >> 4;
	if (nybble <= 9)
		UART_TX(nybble + '0');
	else
		UART_TX(nybble + 'a' - 10);
	nybble = val & 15;
	if (nybble <= 9)
		UART_TX(nybble + '0');
	else
		UART_TX(nybble + 'a' - 10);
	UART_FLUSH();
}

int main(void) {
	// flush previous output, especially xmodem
	puts("\r\n\r\nRAMcheck\r\n");
	
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
		putx(addr);
		puts(":");
		if (addr == 0x84000000 /* nanoboot */
				|| addr == 0x80100000 /* ramcheck itself */
				|| addr == 0x87e00000 /* U-Boot's PCI DMA area */
			) {
			// that's us; don't shoot ourselves!
			puts(" skipped\r\n");
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
				puts("\r\n\t!");
				putx((uint32_t) &block[k]);
				puts(" ");
				put8(block[k]);
				puts(" ");
				put8(x);
				err = 1;
				global_err = 1;
			}
		}
		
		// status
		if (!err)
			puts(" OK");
		puts("\r\n");
	}
	if (global_err)
		puts("WARNING: errors detected! check log for details\r\n");
	else
		puts("all memory OK\r\n");
	// return to nanoboot. for U-Boot, this crashes the system, because
	// U-Boot has been overwritten by ramcheck.
	return 0;
}
