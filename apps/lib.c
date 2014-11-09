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

void putx32(uint32_t val) {
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

void putx16(uint16_t val) {
	for (int i = 0; i < 4; i++) {
		uint16_t nybble = val >> 12;
		if (nybble <= 9)
			UART_TX(nybble + '0');
		else
			UART_TX(nybble + 'a' - 10);
		val <<= 4;
	}
	UART_FLUSH();
}

void putx8(uint8_t val) {
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

void putd(uint32_t val) {
	int lz = 1;
	for (int i = 1000000000; i; i /= 10) {
		uint32_t digit = (val / i) % 10;
		if (digit != 0 || !lz) {
			lz = 0;
			UART_TX(digit + '0');
		}
	}
	if (lz)
		UART_TX('0');
	UART_FLUSH();
}
