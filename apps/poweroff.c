#include <stdint.h>
#include <hw.h>

#include "lib.c"

#define POWER_CONTROL *((volatile uint32_t *) 0xbbf04010)
#define POWER_DOWN_ARM     (0x10000 << 12)
#define POWER_DOWN_REQUEST (0x10001 << 12)

int main(void) {
	printf("\n*** system power down ***\n");
	uint32_t timer = TIMER_START();
	POWER_CONTROL = POWER_DOWN_ARM;
	while (!TIMER_TIMED_OUT(timer, TIMER_TIMEOUT(2)));
	POWER_CONTROL = POWER_DOWN_REQUEST;
	while (1);
}
