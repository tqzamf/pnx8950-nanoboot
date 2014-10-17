#include <stdint.h>
#include "cache.h"

extern int main(void);

void _start(void) __attribute__((section(".start")));
void _start(void) {
	// U-Boot doesn't properly flush the cache, so we have to be called
	// in uncached memory, which is slow. to avoid this, we just flush
	// the cache ourselves here and relocate to cached memory.
	// nanoboot does properly flush the cache, so in that case this code
	// is unnecessary. it doen't hurt, however.
	cache_flush();
	uint32_t _main = (uint32_t) main;
	_main &= ~0x20000000;
	void (*__main)(void) = (void *) _main;
	__main();
}
