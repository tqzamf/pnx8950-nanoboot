#include <stdint.h>
#include "hw.h"
#include "ecc.c"

static volatile uint32_t *nand_base uninitialized;

export void nand_init(void) {
	nand_base = (void *) (FLASH_BASE - 256);
}

export int16_t nand_get_block(uint8_t *base, int expect_header) {
	nand_base += 256/4;
	uint8_t block = ((uint32_t) nand_base) >> 8;
	uint8_t halfpage = block & 0x01;

	if (SW11_STATUS() != 0)
		// skip NAND reading if SW1.1 = on, to allow recovery from an
		// image that loads properly but crashes.
		return 0;

	// end of file! we read the entire flash and found nothing...
	// the core will generate an appropriate error message.
	if (nand_base > FLASH_END)
		return 0;

	if ((block & 0x3f) == 0x3f) {
		if (expect_header)
			UART_TX('.');
		else
			UART_TX('#');
		// timing:
		// XIO takes ~20s to read 4096*16 pages, ie. ~3200 pages/sec.
		// UART at 38.4kbaud can send ~4k chars/sec.
		// thus we don't need to flush, ever. however we need some of
		// that UART bandwidth for error reporting. theoretically, there
		// can be up to 2 errors per page, but in reality there usually
		// are none at all.
		// allowing for a generous ~800 chars/sec progress reporting, we
		// can send an error every second half-page without overwhelming
		// the UART. this corresponds to 64k errors across the device.
		// for comparison, the point where two errors are 99% likely to
		// fall into the same half-page, yielding an uncorrectable error,
		// is at 1099 errors across the device, for a safety margin of
		// ~59x.
		// so the progress output is limited more by terminal scroll rate
		// than by UART speed. U-Boot takes ~256k, or 512 half-pages.
		// printing a dot every 64 pages yields a comfortable 16 dots
		// for loading all of U-Boot, and a convenient single space for
		// the FlashReader preceding it.
	}

	if (halfpage == 0) {
		// copy flash data to RAM, ie. copy the full 512-byte page and
		// all OOB bytes used for ECC. this is essentially a 520-byte DMA
		// operation.
		// select the first half-page and read its first 4 bytes using a
		// full command-and-address cycle. this is really just meant to
		// load the registers in the flash chip; the bytes fall out as a
		// byproduct.
		XIO_SELECT_BLOCK(0);
		uint32_t *buffer = ((uint32_t *) base);
		buffer[0] = *nand_base;
		// address and command are now set; no need to send them again
		// for each byte. the flash will provide bytes sequentially until
		// the end of the current page.
		// not sending the address makes reads much faster because it
		// saves ~80% of the bus-level read/write accesses to the flash,
		// and probably also avoids repeating the page read operation by
		// the flash itself. it also means that we don't have to
		// increment the nand address because it isn't sent anyway.
		XIO_SELECT_SEQUENTIAL();
		for (uint32_t pos = 1; pos < 512/4 + 8/4; pos++)
			buffer[pos] = *nand_base;
	}

	// read and decode OOB bytes. the layout in the OOB area is:
	// L0 L1 L2 U0 BB xx U1 U2
	uint8_t bbm, ecc0, ecc1, ecc2;
	if (halfpage == 0) {
		ecc0 = base[512 + 0];
		ecc1 = base[512 + 1];
		ecc2 = base[512 + 2];
		bbm = base[512 + 5];
	} else {
		ecc0 = base[256 + 3];
		ecc1 = base[256 + 6];
		ecc2 = base[256 + 7];
		bbm = base[256 + 5];
	}
	if (bbm != 0xff)
		// OOB area byte 5 programmed: bad block; next one, please!
		return -1;
	uint32_t ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);

	// try to correct any errors that might have crept into the NAND
	// data. ECC can correct any single-bit error, but if the block
	// isn't refreshed soo, it might develop an uncorrectable 2-bit
	// error. therefore, emit a warning even on single-bit errors.
	uint32_t num_errors = ecc_correct(base, ecc);
	switch (num_errors) {
	case 1:
		UART_TX('E');
		// fallthrough!
	case 0:
		if (expect_header && !is_header(base))
			// expecting a header block, but this isn't one yet.
			// next one!
			return -1;
		return 256;
	
	default:
		UART_TX('U');

		// uncorrectable error in the data area. that's a fatal error
		// condition that we cannot recover from.
		if (!expect_header)
			return 0;
		// if we are still waiting for the header, we want the main loop
		// to continue calling us until we finally find a valid header,
		// or run out of pages to read.
		return -1;
	}
}
