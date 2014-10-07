#include <stdint.h>
#include "hw.h"

static volatile uint8_t *nand_base;
static uint8_t nand_status;

export void nand_init(void) {
	nand_base = FLASH_BASE - 256;
	nand_status = '_';
}

static inline void nand_set_status(uint8_t status) {
	if (nand_status > status)
		nand_status = status;
}

export int16_t nand_get_block(uint8_t *base, int in_header) {
	nand_base += 256;
	uint8_t block = ((uint32_t) nand_base) >> 8;
	uint8_t halfpage = block & 0x01;

	if ((block & 0x1f) == 0) {
		UART_TX(nand_status);
		nand_status = '_';
		// TODO should be timed to not need flushing, else debug output slows down loading!
		UART_FLUSH();
	}

	// end of file! we read 64MB of data and found nothing...
	if (nand_base > FLASH_END) {
		UART_TX('E');
		return -2;
	}

	// read OOB area first. selecting it is intentionally done in such a
	// way that it re-selects full command-and-address cycles.
	XIO_SELECT_OOB();
	// bad block; next one, please!
	if (nand_base[5] != 0xff) {
		nand_set_status('-');
		return -1;
	}

	// read ECC bytes
	uint8_t b0, b1, b2;
	if (halfpage == 0) {
		b0 = nand_base[0];
		b1 = nand_base[1];
		b2 = nand_base[2];
	} else {
		b0 = nand_base[3];
		b1 = nand_base[6];
		b2 = nand_base[7];
	}

	// select the correct half-page and read its first 4 bytes using full
	// command-and-address cycles. this is really just meant to load the
	// registers in the flash chip; the byte falls out as a byproduct.
	XIO_SELECT_BLOCK(halfpage);
	volatile uint32_t *nand_page = ((volatile uint32_t *) nand_base);
	uint32_t *buffer = ((uint32_t *) base);
	buffer[0] = *nand_page;
	// address and command are now set; no need to send them again for
	// each byte. the flash will provide bytes sequentially until the
	// end of the current page.
	// not sending the address makes reads much faster because it saves
	// ~80% of the read/write accesses to the flash, and probably also
	// avoids repeating the page read operation by the flash itself.
	// it also means that we don't have to increment the nand address
	// because it isn't sent anyway.
	XIO_SELECT_SEQUENTIAL();
	for (uint32_t pos = 1; pos < 256/4; pos++)
		buffer[pos] = *nand_page;

	for (uint32_t pos = 0; pos < 256; pos++) {
		// TODO calculate ECC
	}

	// TODO check and correct ECC
	// nand_set_status('.'); // no error (only outside header!)
	// nand_set_status('!'); // error corrected
	// nand_set_status(' '); return -1; // uncorrectable in header
	// UART_TX('U'); return -2; // uncorrectable outside header

	if (!in_header)
		nand_set_status('.');
	return 256;
}
