#include <stdint.h>

#define export static inline

#include "hw.h"
#include "cache.h"
#include "xmodem.c"

#define IMAGE_LOAD_ADDR     (UNCACHED_BASE | 0x01000000)
#define IMAGE_ENTRY_POINT   (DRAM_BASE | 0x01000000)
#define IMAGE_HEADER_SIZE 0x200
#define IMAGE_HEADER_SIGNATURE 0x57434530
struct __attribute__((packed)) image_header {
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
	// disable cache so that the image runs uncached. this simplifies
	// coherency and allows it to mess with the caches. the LED control
	// serves the dual purpose of filling any hazard slots that might
	// exist.
	cache_disable();
	LEDS_SET(1, 0);

	// call image at 0x81000000, unsing an absolute-address jump.
	void (*image)(void) = (void *) IMAGE_ENTRY_POINT;
	asm volatile("" : "=r" (image) : "r" (image));
	image();
	
	// we only ever get here when the image returns, which it doesn't
	// generally do. however, we permit returning to the bootloader so
	// that images can extend the bootloader.
	// re-enable the cache for performance. might not come into effect
	// immediately, but will eventually. if the image wrote to the
	// instruction stream, the system is probably about to crash anyway.
	cache_enable();
	return;
}

#define INFINITY (0xffff0000)
static inline void load_and_call_image(void) {
	#define HEADER_BASE ((void *) (IMAGE_LOAD_ADDR - IMAGE_HEADER_SIZE))
	struct image_header *header = (void *) HEADER_BASE;
	uint8_t *base = HEADER_BASE;
	uint8_t *data_end = (void *) INFINITY;
	
	while (((uint32_t) base) < (((uint32_t) data_end) + 128)) {
		// we intentionally try to read "beyond" end of file. this is
		// harmless on flash, and gives xmodem a chance to end the
		// transfer cleanly.

		int16_t res = get_block(base);
		if (res == 0)
			// end of file. stop reading; there is nothing more to read.
			break;
		if (res < 0)
			// invalid block. try again.
			continue;
		
		if (((uint32_t) data_end) != INFINITY)
			// got a good data block; next one, please!
			base += res;
		else if (header->signature1 == IMAGE_HEADER_SIGNATURE
					&& header->signature2 == IMAGE_HEADER_SIGNATURE) {
			// found a valid header. start getting data.
			data_end = (void *) (IMAGE_LOAD_ADDR + header->length);
			base += res;
		}
	}

	// if the image is complete, try calling it. don't call an incomplete
	// image; try to receive a new one instead.
	if (((uint32_t) base) >= ((uint32_t) data_end))
		call_image();
}

void _main(void) __attribute__((noreturn, section(".init")));
void _main(void) {
	for (;;) {
		load_and_call_image();
		
		LEDS_SET(0, 1);
		xmodem_enabled = 1;
		xmodem_init();
	}
}
