#include <stdint.h>

#define export static inline

#include "hw.h"
#include "cache.h"
#include "header.h"
#include "xmodem.c"
#include "nand.c"

static uint8_t xmodem_enabled uninitialized;

static inline uint32_t get_block(uint8_t *base, int expect_header) {
	if (xmodem_enabled)
		return xmodem_get_block(base, expect_header);
	else
		return nand_get_block(base, expect_header);
}

static inline void call_image(uint32_t addr) {
	// flush cache and run image cached, for ideal performance. the LED
	// control serves the dual purpose of filling any hazard slots that
	// might exist.
	cache_flush();
	LEDS_SET(0, 1);

	// call image at its entry point, using an absolute-address jump.
	void (*image)(void) = (void *) (addr | DRAM_BASE);
	image();

	// we only ever get here when the image returns, which it doesn't
	// generally do. however, we permit returning to the bootloader so
	// that images can extend the bootloader.
	return;
}

#define INFINITY (0xffff0000)
static inline void load_and_call_image(void) {
	struct image_header *header = (void *) IMAGE_HEADER_BASE;
	uint8_t *base = IMAGE_HEADER_BASE;
	uint8_t *data_end = (void *) INFINITY;

	while (((uint32_t) base) < (((uint32_t) data_end) + 128)) {
		// we intentionally try to read "beyond" end of file. this is
		// harmless on flash, and gives xmodem a chance to end the
		// transfer cleanly.
		int expect_header = ((uint32_t) data_end) == INFINITY;

		int16_t res = get_block(base, expect_header);
		if (res == 0)
			// serious error or end of file condition. don't retry.
			// if this happens while waiting for the header, the normal
			// fallback will start receiving an image over xmodem,
			// thereby retrying xmodem but not flash reads.
			break;
		if (res < 0)
			// invalid block. try again.
			continue;

		if (expect_header)
			// found a valid header. start getting data.
			data_end = (void *) (IMAGE_LOAD_ADDR + header->length);
		base += res;
	}

	// add a linebreak to separate nanoboot progress output from the next
	// stage. flush output before, because the UART drops new bytes if
	// its buffer is full.
	UART_FLUSH();
	UART_TX('\r');
	UART_TX('\n');
	// if the image is complete, try calling it. don't call an incomplete
	// image; try to receive a new one instead. however, do try to call
	// a complete image even on fatal errors. in that case, the error
	// happened after the end of the image, and so is harmless.
	if (((uint32_t) base) >= ((uint32_t) data_end))
		call_image(header->entrypoint);
}

void _main(void) __attribute__((noreturn, section(".init")));
void _main(void) {
	xmodem_enabled = 0;
	nand_init();
	UART_TX('\r');
	UART_TX('\n');

	for (;;) {
		load_and_call_image();

		// send "prompt" and turn on the red LED to signal that
		// nanoboot is ready for Xmodem data.
		LEDS_SET(1, 0);
		UART_TX('X');
		xmodem_enabled = 1;
		xmodem_init();
	}
}
