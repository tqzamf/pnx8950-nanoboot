#include <stdint.h>
#include <hw.h>
#include <cache.h>

#include "lib.c"

int main(void) {
	// flush previous output, especially xmodem
	printf("\nMIPS CP0 register dump\n");
	
	printf(" R");
	#define SEL(sel) \
		printf("%9d", sel);
	#define SEL2(sel) \
		SEL(sel+0); \
		SEL(sel+1); \
		SEL(sel+2); \
		SEL(sel+3);
	SEL2(0);
	SEL2(4);
	printf("\n");
	
	#define DUMP(reg,sel) \
		printf(" %08x", MFC0(reg, sel));
	#define DUMPS(reg) \
		printf("%2d", reg); \
		DUMP(reg, 0); \
		DUMP(reg, 1); \
		DUMP(reg, 2); \
		DUMP(reg, 3); \
		DUMP(reg, 4); \
		DUMP(reg, 5); \
		DUMP(reg, 6); \
		DUMP(reg, 7); \
		printf("\n");
	#define DUMPS2(reg) \
		DUMPS(reg+0); \
		DUMPS(reg+1); \
		DUMPS(reg+2); \
		DUMPS(reg+3);
	#define DUMPS3(reg) \
		DUMPS2(reg+0); \
		DUMPS2(reg+4); \
		DUMPS2(reg+8); \
		DUMPS2(reg+12);
	DUMPS3(0)
	DUMPS3(16)

	return 0;
}
