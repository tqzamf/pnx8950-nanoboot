#include <stdint.h>
#include "../cache.h"

#define RESET (*((uint32_t *) 0xbbe60000))
#define ASSERT_SYS_RESET  1
#define RELEASE_SYS_RESET 2

#define DECLARE(name) \
	volatile uint32_t *name##_LOWER = (void *) name##_BASE; \
	volatile uint32_t *name##_UPPER = (void *) (name##_BASE + 0x10000); \
	asm("" : "=r" (name##_LOWER) : "r" (name##_LOWER)); \
	asm("" : "=r" (name##_UPPER) : "r" (name##_UPPER));
#define SET(name, mod, off, val) \
	do { \
		if (0x##mod >= 0x8) \
			name##_UPPER[(0x##mod##off - 0x10000) >> 2] = 0x##val; \
		else \
			name##_LOWER[0x##mod##off >> 2] = 0x##val; \
	} while (0)

#define BBE4_BASE 0xbbe40000
#define DECLARE_CLOCK DECLARE(BBE4)
#define DECLARE_UART1
#define DECLARE_UART2
#define DECLARE_GLOBAL2
#define BBE6_BASE 0xbbe60000
#define DECLARE_GLOBAL1 DECLARE(BBE6)
#define CLOCK(x, y) SET(BBE4,7,x, y)
#define GLOBAL1(x, y) SET(BBE6,3,x, y)
#define GLOBAL2(x, y) SET(BBE4,D,x, y)
#define UART1(x, y) SET(BBE4,A,x, y)
#define UART2(x, y) SET(BBE4,B,x, y)

extern void __image_end(void);
extern void __image_start(void);

static inline void init(void) {
	// reset all PCI devices, because U-Boot leaves them in a state that
	// Windows CE cannot handle. the system reset is a fairly radical way
	// of doing that, but it's a lot easier than writing a PCI driver.
	RESET = ASSERT_SYS_RESET;

	// clear memory because WinCE expects some areas to be zeroed. leave a
	// WinCE-shaped hole so we don't overwrite ourselves. no need to skip
	// the stack though; that doesn't contain anything useful anyway.
	#define INIT(from, to) \
		for (uint32_t addr = (from); addr != (to); addr += 16) { \
			uint32_t *word = (uint32_t *) addr; \
			word[0] = 0; word[1] = 0; word[2] = 0; word[3] = 0; \
		}
	INIT(0x80000000, (uint32_t) __image_start);
	INIT((uint32_t) __image_end, 0x88000000);
	// not flushing cache here; it's pointless. data accesses to the
	// cleared regions go through cache, which is self-consistent. and
	// executing from the cleared regions is probably incorrect anyway.

	// CMEM registers, as set up by FlashReader. U-Boot has a different
	// layout.
	MTC0_CMEM(0, 0x1be00000, CMEM_SIZE_2M);
	MTC0_CMEM(1, 0x10000000, CMEM_SIZE_128M);
	MTC0_CMEM(2, 0x18000000, CMEM_SIZE_64M);
	MTC0_CMEM(3, 0x1c000000, CMEM_SIZE_64M);
	// stop the timers, enable TLB, disable static mapping. nanoboot and
	// U-Boot have the TLB disabled; Windows CE wants it enabled but
	// unlike Linux doesn't enable it itself.
	// the timers are just for good measure; they should be stopped anyway.
	uint32_t configPR = MFC0_CONFIGPR();
	configPR |= CPR_T1 | CPR_T2 | CPR_T3;
	configPR |= CPR_TLB;
	configPR &= ~CPR_MAP;
	configPR &= CPR_VALID;
	MTC0_CONFIGPR(configPR);

	// release system reset. memory clearing took a while, so the PCI
	// devices should now be in a clean initial state that Windows CE
	// can handle. the hardware init gives them some time to come out of
	// reset before Windows CE wants to access them.
	RESET = RELEASE_SYS_RESET;

	// perform some extra hardware init that the original bootscript does
	// but nanoboot omits.
	// start some extra (AV system) clocks
	DECLARE_CLOCK;
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
	CLOCK(714, 00000003); // I²C HP
	CLOCK(718, 00000003); // I²C Fast
	// some UART setup normally done by FlashReader
	DECLARE_UART2;
	UART2(000, 01060000); // flush buffers and configure for 8-bit
	UART2(fe8, 00001fff); // clear any pending interrupts
	UART2(004, 00000003); // setup modem & flow control lines
	UART2(008, 00000005); // set baud rate to 38.4k, as by FlashReader
	// UART1 is simpler. modem / flow control isn't connected, and we
	// leave the baud rate as it is, even if it's wrong. UART1 is used
	// for the kernel log only, so it's actually better if the baud rate
	// doesn't have to be changed on the other end.
	DECLARE_UART1;
	UART1(000, 01060000); // flush / 8-bit
	UART1(fe8, 00001fff); // interrupts
	// pretend we're the original bootscript.
	DECLARE_GLOBAL1;
	GLOBAL1(500, 00130001);
	GLOBAL1(504, 00000006);
	GLOBAL1(508, 00010014);
	// misc registers (for good measure; not really required)
	DECLARE_GLOBAL2;
	GLOBAL2(60c, 00000001); // probably VIP control
	GLOBAL2(050, 00000001); // PCI INTA enable
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

void _start(void) __attribute__((section(".start")));
void _start(void) {
	// flush cache, because U-Boot doesn't do that itself. there could
	// be stale data in the cache without this flush.
	cache_flush();
	// set up a safe stack that won't overwrite anything important
	asm volatile("move $sp, %0" :: "r" (_start) : "memory");

	// perform some hardware initialization that nanoboot omits
	init();
	// call the image
	image_main();
}
