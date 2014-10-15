#include <stdint.h>

extern int main(void);

void _start(void) __attribute__((section(".start")));
void _start(void) {
	main();
}
