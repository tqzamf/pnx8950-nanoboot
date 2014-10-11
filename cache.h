#ifndef _CACHE_H
#define _CACHE_H

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
#define ICACHE_INDEX_INVALIDATE(index) \
	do { \
		asm volatile( \
			"cache 0x00, 0(%z0)\n\t" \
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
#define DCACHE_INDEX_INVALIDATE_WRITEBACK(index) \
	do { \
		asm volatile( \
			"cache 0x01, 0(%z0)\n\t" \
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
#define UNCACHED_BASE 0xA0000000

static inline void cache_enable(void) {
	MTC0_CONFIG(CFG_CACHEABLE);
}

static inline void cache_disable(void) {
	MTC0_CONFIG(CFG_UNCACHED);
}

static inline void cache_init(void) {
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
}

static inline void cache_flush(void) {
	// invalidate cache (instruction and data). do it in a single loop
	// if possible to spave some space.
#if (ICACHE_ENTRIES == DCACHE_ENTRIES) && (ICACHE_LINE == DCACHE_LINE)
	for (uint32_t index = 0; index < ICACHE_LINE * ICACHE_ENTRIES - 1;
			index += ICACHE_LINE) {
		ICACHE_INDEX_INVALIDATE(index);
		DCACHE_INDEX_INVALIDATE_WRITEBACK(index);
	}
#else
	for (uint32_t index = 0; index < ICACHE_LINE * ICACHE_ENTRIES - 1;
			index += ICACHE_LINE)
		ICACHE_INDEX_INVALIDATE(index);
	for (uint32_t index = 0; index < DCACHE_LINE * DCACHE_ENTRIES - 1;
			index += DCACHE_LINE)
		DCACHE_INDEX_INVALIDATE_WRITEBACK(index);
#endif
}

#define MTC0_CMEM(x, base, size, enable) \
	MTC0(22, (x) + 4, (base) | ((size) << 1) | ((enable) ? 1 : 0))
#define CMEM_SIZE_1M   0
#define CMEM_SIZE_2M   1
#define CMEM_SIZE_4M   2
#define CMEM_SIZE_8M   3
#define CMEM_SIZE_16M  4
#define CMEM_SIZE_32M  5
#define CMEM_SIZE_64M  6
#define CMEM_SIZE_128M 7
#define CMEM_XIO_BASE  0x10000000
#define CMEM_XIO_SIZE  CMEM_SIZE_128M
#define CMEM_MMIO_BASE 0x1BE00000
#define CMEM_MMIO_SIZE CMEM_SIZE_2M
#define CMEM_PCI_BASE  0x1c000000
#define CMEM_PCI_SIZE  CMEM_SIZE_32M

#endif
