#include <stdint.h>

#define _GPIO_BASE 0xBBF04000
#define _GPIO_TIMER (*((volatile uint32_t *) (_GPIO_BASE + 0x0C0)))
#define _GPIO_LEDS1 (*((volatile uint32_t *) (_GPIO_BASE + 0x01C)))

void _start(void) {
	while (1) {
		uint32_t timer = _GPIO_TIMER;
		_GPIO_LEDS1 = 0x11000000 | ((timer & 0x0200000) ? 0 : 0x1000) | ((timer & 0x0100000) ? 0 : 0x100); \
	}
}

#define IMAGE_HEADER_SIGNATURE 0x57434530
struct __attribute__((packed)) image_header {
	uint32_t signature1;
	uint32_t length;
	uint32_t signature2;
	uint32_t unknown;
} header __attribute__((section(".header"))) = {
	IMAGE_HEADER_SIGNATURE,
	512,
	IMAGE_HEADER_SIGNATURE,
	0
};
