#include <stdint.h>

#define export static inline

#include "hw.h"
#include "xmodem.c"

#define IMAGE_HEADER_SIZE 0x200
#define IMAGE_HEADER_SIGNATURE 0x30454357
struct image_header {
	uint32_t signature1;
	uint32_t length;
	uint32_t signature2;
	uint32_t unknown;
};

static uint8_t xmodem_enabled;

static inline uint32_t get_block(uint8_t *base) {
	if (xmodem_enabled)
		return xmodem_get_block(base);
	else
		// dummy: read from NAND
		return 0;
}

static inline void call_image(void) {
	// disable cache
	// call image at 0x81000000
	// re-enable cache (in case it does return)
	return;
}

static inline void load_and_call_image(void) {
	uint8_t *const load_base = (void *) (0xA1000000 - IMAGE_HEADER_SIZE);
	struct image_header *header = (void *) load_base;
	uint8_t *base = load_base;
	uint32_t wait_for_header = 1;
	
	while (wait_for_header || (base - load_base) < header->length) {
		uint32_t res = get_block(base);
		if (res == 0)
			// end of file. stop reading when there is nothing more to
			// read.
			break;
		if (res < 0)
			// invalid block. try again.
			continue;
		
		if (!wait_for_header)
			// got a good data block; next one, please!
			base += res;
		else if (header->signature1 == IMAGE_HEADER_SIGNATURE
					&& header->signature2 == IMAGE_HEADER_SIGNATURE)
			// found a valid header. start getting data.
			wait_for_header = 0;
	}

	// if the image is complete, try calling it. don't call an incomplete
	// image; try to receive a new one instead.
	if ((base - load_base) >= header->length)
		call_image();
}

void _main(void) __attribute__((noreturn, section(".init")));
void _main(void) {
	for (;;) {
		load_and_call_image();
		
		xmodem_enabled = 1;
		xmodem_init();
	}
}
