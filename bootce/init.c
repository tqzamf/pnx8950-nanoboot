#include <stdint.h>
#include "../cache.h"

#define RESET (*((uint32_t *) 0xbbe60000))
#define ASSERT_SYS_RESET  1
#define RELEASE_SYS_RESET 2
#define CLOCK(x, y) *((uint32_t *) 0xbbe47##x) = 0x##y;
#define GLOBAL1(x, y) *((uint32_t *) 0xbbe63##x) = 0x##y;
#define GLOBAL2(x, y) *((uint32_t *) 0xbbe4d##x) = 0x##y;
#define UART1(x, y) *((uint32_t *) 0xbbe4a##x) = 0x##y;
#define UART2(x, y) *((uint32_t *) 0xbbe4b##x) = 0x##y;

extern void _start(void);
extern void __image_end(void);
#define STACK_SIZE 0xf000

static inline void init(void) {
	// reset all PCI devices, because U-Boot leaves them in a state that
	// Windows CE cannot handle. the system reset is a fairly radical way
	// of doing that, but it's a lot easier than writing a PCI driver.
	RESET = ASSERT_SYS_RESET;
	// clear memory. WinCE expects some areas to be zeroed. leave a
	// WinCE-shaped hole so we don't overwrite ourselves, and don't kill
	// our stack either.
	#define INIT(from, to) \
		for (uint32_t addr = (from); addr != (to); addr += 16) { \
			uint32_t *word = (uint32_t *) addr; \
			word[0] = 0; \
			word[1] = 0; \
			word[2] = 0; \
			word[3] = 0; \
		}
	INIT(0x80000000, ((uint32_t) _start) - STACK_SIZE);
	INIT((uint32_t) __image_end, 0x88000000);
	cache_flush();
	// CMEM registers, as set up by FlashReader. U-Boot has a different
	// layout.
	MTC0_CMEM(0, 0x1be00000, CMEM_SIZE_2M);
	MTC0_CMEM(1, 0x10000000, CMEM_SIZE_128M);
	MTC0_CMEM(2, 0x18000000, CMEM_SIZE_64M);
	MTC0_CMEM(3, 0x1c000000, CMEM_SIZE_64M);
	// start some extra clocks that nanoboot omits
	CLOCK(004, 00b40904); // TM32 1
	CLOCK(008, 3828050c); // QVCP 1
	CLOCK(00c, 3828050c); // QVCP 2
	CLOCK(048, 00b40904); // TM32 2
	CLOCK(204, 00000103); // TM32 1
	CLOCK(208, 00000103); // TM32 2
	CLOCK(a00, 00000003); // QVCP 1
	CLOCK(a04, 0000000b); // QVCP 1
	CLOCK(a08, 00000003); // QVCP 2
	CLOCK(a0c, 0000000b); // QVCP 2
	CLOCK(800, 00000023); // MSP 1
	CLOCK(804, 00000023); // MSP 2
	CLOCK(500, 00000073); // MBS 1
	CLOCK(504, 00000073); // MBS 3
	CLOCK(708, 00000003); // unknown
	// some UART setup normally done by FlashReader
	UART1(000, 00040000);
	UART1(000, 00020000);
	UART1(fe8, 000000ff);
	UART1(000, 01000000);
	UART1(004, 00000003);
	UART1(008, 00000005);
	UART2(000, 00040000);
	UART2(000, 00020000);
	UART2(fe8, 000000ff);
	UART2(000, 01000000);
	UART2(004, 00000003);
	UART2(008, 00000005);
	// misc registers (for good measure; not really required)
	GLOBAL2(60c, 00000001); // probably VIP control
	GLOBAL2(050, 00000001); // PCI INTA enable
	// release system reset. the PCI devices should now be in a clean
	// initial state that Windows CE can handle.
	RESET = RELEASE_SYS_RESET;
	// stop the timers, enable TLB, disable static mapping. nanoboot and
	// U-Boot have the TLB disabled; Windows CE wants it enabled but
	// unlike Linux doesn't enable it itself. the timers are just for
	// good measure; they should be stopped anyway.
	uint32_t configPR = MFC0_CONFIGPR();
	configPR |= CPR_T1 | CPR_T2 | CPR_T3;
	configPR |= CPR_TLB;
	configPR &= ~CPR_MAP;
	configPR &= CPR_VALID;
	MTC0_CONFIGPR(configPR);
	// pretend we're the original bootscript. this is at the end so we
	// don't do a mtc0 in the branch delay slot; that appears a bit TOO
	// bold.
	GLOBAL1(500, 00130001);
	GLOBAL1(504, 00000006);
	GLOBAL1(508, 00010014);
	// manufacturer ID, hardware ID and board UUID, as set by the
	// original bootschript
	GLOBAL2(500, ffffffff);
	GLOBAL2(504, ffffffff);
	GLOBAL2(508, ffffffff);
	GLOBAL2(50c, ffffffff);
	GLOBAL2(510, ffffffff);
	GLOBAL2(514, ffffffff);
	GLOBAL2(518, 33533046);
	GLOBAL2(51c, 54476d62);
}

extern void image_main(void) __attribute__((noreturn));

int main(void) {
	// perform some hardware initialization that nanoboot omits
	init();
	// call the image
	image_main();
}

void _start(void) __attribute__((section(".start")));
void _start(void) {
	// flush cache, because U-Boot doesn't do that itself. there could
	// be stale data in the cache without this flush.
	cache_flush();
	// set up a safe stack that won't be overwritten by memory clearing
	asm volatile("move $sp, %0" :: "r" (_start) : "memory");
	// jump to cached memory. runs waaay faster.
	uint32_t main_addr = UNCACHED_BASE | (uint32_t) main;
	int (* main_cached)(void) = (void *) (main_addr - UNCACHED_BASE + DRAM_BASE);
	main_cached();
}
