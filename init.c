#include <stdint.h>

#define MFC0(reg, sel) \
	({ uint32_t res; \
		asm volatile( \
			"mfc0 %0, $%1, %2\n\t" \
			: "=r" (res) \
			: "i" (reg), "i" (sel)); \
		res; \
	})
#define MTC0(reg, sel, value) \
	do { \
		asm volatile( \
			"mtc0 %z0, $%1, %2\n\t" \
			: \
			: "Jr" ((unsigned int)(value)), "i" (reg), "i" (sel)); \
	} while (0)

#define ICACHE_LINE 32
#define ICACHE_ENTRIES (256*2)
#define ICACHE_INDEX_STORE_TAG(index) \
	do { \
		asm volatile( \
			"cache 0x08, 0(%z0)\n\t" \
			: \
			: "Jr" ((unsigned int)(index))); \
	} while (0)
#define DCACHE_LINE 32
#define DCACHE_ENTRIES (128*4)
#define DCACHE_INDEX_STORE_TAG(index) \
	do { \
		asm volatile( \
			"cache 0x09, 0(%z0)\n\t" \
			: \
			: "Jr" ((unsigned int)(index))); \
	} while (0)

#define MTC0_CONFIG(value)   MTC0(16, 0, value)
#define CFG_UNCACHED  2
#define CFG_CACHEABLE 3
#define MTC0_CAUSE(value)    MTC0(13, 0, value)
#define MFC0_STATUS()        MFC0(12, 0)
#define MTC0_STATUS(value)   MTC0(12, 0, value)
#define ST0_CU0       0x10000000
#define ST0_IE        0x00000001
#define ST0_EXL       0x00000002
#define ST0_ERL       0x00000004
#define ST0_KSU       0x00000018
#define MFC0_CONFIGPR()      MFC0(16, 7)
#define MTC0_CONFIGPR(value) MTC0(16, 7, value)
#define CPR_TLB       (1 << 19)
#define CPR_MAP       (1 << 20)
#define CPR_T1        (1 << 3)
#define CPR_T2        (1 << 4)
#define CPR_T3        (1 << 5)
#define CPR_VALID     0x3F1F41FF    /* valid bits to write to ConfigPR */
#define MTC0_TAGLO(value)    MTC0(28, 0, value)
#define MTC0_TAGHI(value)    MTC0(29, 0, value)
#define MTC0_DTAGLO(value)   MTC0(28, 2, value)
#define MTC0_DTAGHI(value)   MTC0(29, 2, value)
#define DRAM_BASE     0x80000000

int main(void) __attribute__((noreturn));

void _init(void) __attribute__((section(".init")));
void _init(void) {
	// enable CP0 and disable all interrupts. zero the rest of the
	// register as well to get well-defined defaults.
	MTC0_STATUS(ST0_CU0);
	// clear all interrupt flags
	MTC0_CAUSE(0);
	
	// disable cache. it should be disabled by default, but make sure it
	// really is. if we initialize it while it's enabled, we risk massive
	// cache corruption.
	MTC0_CONFIG(CFG_UNCACHED);
	MTC0_TAGLO(0);
	MTC0_TAGHI(0);
	// these two are probably unnecessary unless there are separate D
	// cache tag registers.
	MTC0_DTAGLO(0);
	MTC0_DTAGHI(0);
	
	// disable TLB and static mapping, and stop the timers. 
	// also fills any hazard slots for cache disable.
	uint32_t configPR = MFC0_CONFIGPR();
	configPR &= CPR_VALID;
	configPR &= ~(CPR_TLB | CPR_MAP);
	configPR |= CPR_T1 | CPR_T2 | CPR_T3;
	MTC0_CONFIGPR(configPR);

	// initialize cache (instruction and data). do it in a single loop
	// if possible to spave some space.
#if (ICACHE_ENTRIES == DCACHE_ENTRIES) && (ICACHE_LINE == DCACHE_LINE)
	for (uint32_t index = 0; index < ICACHE_LINE * ICACHE_ENTRIES - 1;
			index += ICACHE_LINE) {
		ICACHE_INDEX_STORE_TAG(index);
		DCACHE_INDEX_STORE_TAG(index);
	}
#else
	for (uint32_t index = 0; index < ICACHE_LINE * ICACHE_ENTRIES - 1;
			index += ICACHE_LINE)
		ICACHE_INDEX_STORE_TAG(index);
	for (uint32_t index = 0; index < DCACHE_LINE * DCACHE_ENTRIES - 1;
			index += DCACHE_LINE)
		DCACHE_INDEX_STORE_TAG(index);
#endif
	
	// perform a dummy read to initialize some latches, or something.
	// this probably isn't needed but we have some hazard cycles to kill
	// anyway.
	*((volatile uint32_t *) DRAM_BASE);
	
	// enable caches. instruction fetches might not immediately use the
	// cache, but it should work anyway.
	MTC0_CONFIG(CFG_CACHEABLE);
	// don't do the mtc0 in the branch delay slot though; that's a bit
	// TOO bold.
	asm volatile("nop"::);

	// ready to run C code in cached mode. enter main loop.
	main();
}
