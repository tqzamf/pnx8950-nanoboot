#include <stdint.h>

#define export static inline

#include "status.h"
#include "hw.h"
#include "cache.h"
#include "header.h"
#include "xmodem.c"
#include "nand.c"

static inline uint32_t get_block(uint8_t *base, int expect_header, int eof) {
	return nand_get_block(base, expect_header, eof);
}

static inline void call_image(uint32_t addr) {
	// flush cache and run image cached, for ideal performance. the LED
	// control serves the dual purpose of filling any hazard slots that
	// might exist.
	cache_flush();
	LEDS_SET(0, 1);

	// call image at its entry point, using an absolute-address jump.
	void (*image)(void) = (void *) addr;
	image();

	// we only ever get here when the image returns, which it doesn't
	// generally do. however, we permit returning to the bootloader to
	// allow some degree of failsafe; the loader will just continue
	// with the next image.
	return;
}

#define INFINITY (0xffff0000)
static inline void load_and_call_image(void) {
	struct image_header *header = (void *) IMAGE_HEADER_BASE;
	uint32_t base = IMAGE_HEADER_BASE;
	uint32_t data_end = INFINITY;
	int expect_header = 1;

	while (base <= data_end) {
		// we intentionally try to read "beyond" end of file, by running the
		// loop once more if base == data_end. flash detects this and just
		// returns EOF immediately, but xmodem gets a chance to end the
		// transfer cleanly.
		// BOUNDARY CASE: if the image length is less than the file length,
		// so that the xmodem client keeps sending data after the image is
		// complete, that data will go unacknowledged because the image will
		// already be running.

		expect_header = base == IMAGE_HEADER_BASE;
		int eof = base == data_end;
		int16_t res = get_block((void *) base, expect_header, eof);
		if (res < 0)
			// transfer either aborted (STATUS_ABORT) or terminated 
			// successfully (STATUS_EOF), but it doesn't make a
			// difference: if the image is valid and complete, we can
			// call it anyway, because any abort only affected the first
			// block after the image. a true abort during the image
			// leaves an incomplete image, and thus will never try to
			// call it.
			break;

		// correct block received; append the next one.
		// note that invalid blocks just return zero, thus when they are
		// treated as no-errors, they will advance the load address by
		// 0 bytes, ie. retry the current block. this retry re-receives
		// corrupted xmodem blocks and skips bad blocks in flash.
		base += res;

		if (base == IMAGE_HEADER_BASE + IMAGE_HEADER_SIZE) {
			// a full, valid header has been received (all 512 bytes of
			// it). begin receiving image data, starting at the load
			// address, and keep going until the entire image has been
			// received.
			// note that this block is executed exactly once, unless the
			// load address is set to zero. if someone actually does
			// that, the loop will just harmlessly wait for another
			// header.
			base = header->loadaddr | DRAM_BASE;
			data_end = base + header->length;
		}
	}

	// add a linebreak to separate nanoboot progress output from the next
	// stage. also flush output first, because the UART drops new bytes if
	// its buffer is full.
	// this is called after every image load attempt, whether successful
	// or not, thus visually separating the attempts.
	UART_FLUSH();
	UART_TX('\r');
	UART_TX('\n');
	
	// if the image is complete, try calling it, even if a fatal error
	// has been reported: if the image is complete, the error was in the
	// block behind it, and is thus harmless.
	if (base >= data_end && !expect_header)
		call_image(header->loadaddr | DRAM_BASE);
	
	// if the image returns (which is unusual but permitted), continue
	// scanning for and loading the next image.
	return;
}

void _main(void) __attribute__((noreturn, section(".init")));
void _main(void) {
	// initialize flash reader to start of flash. smaller than having
	// a single initialized variable in the .data section.
	nand_init();
	// clear any debris left by a system reboot.
	UART_TX('\r');
	UART_TX('\n');

	for (;;) {
		// reset xmodem status. unnecessary while still reading flash,
		// but harmless.
		xmodem_init();
		// try to load the image
		load_and_call_image();
		// some status output when an image returns or fails to load.
		// this doesn't indicate xmodem readyness, but the LEDs already
		// indicate that to the user and the NAKs indicate it to the
		// xmodem client.
		UART_TX('!');
	}
}
