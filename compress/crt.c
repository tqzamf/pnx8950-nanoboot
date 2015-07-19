#include <stdint.h>

extern void __bss_start(void);
extern void __bss_end(void);

int main(void);

static inline void clear_bss(void) {
	uint32_t *ptr = (void *) __bss_start;
	do {
		ptr[0] = 0;
		ptr[1] = 0;
		ptr[2] = 0;
		ptr[3] = 0;
		ptr += 4;
	} while (ptr <= (uint32_t *) __bss_end);
}

int _start(void) __attribute__((section(".start")));
int _start(void) {
	clear_bss();
	return main();
}
