#include <stdint.h>

#define _GPIO_BASE 0xBBF04000
#define _GPIO_TIMER (*((volatile uint32_t *) (_GPIO_BASE + 0x0C0)))
#define _GPIO_LEDS1 (*((volatile uint32_t *) (_GPIO_BASE + 0x01C)))

int main(void) {
	while (1) {
		uint32_t timer = _GPIO_TIMER;
		_GPIO_LEDS1 = 0x11000000 | ((timer & 0x0200000) ? 0 : 0x1000) | ((timer & 0x0100000) ? 0 : 0x100); \
	}
}
