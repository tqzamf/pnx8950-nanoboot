#ifndef _CRC32_H
#define _CRC32_H

#include <stdint.h>

extern void crc32_init(void);
extern uint32_t crc32(const void *buf, uint32_t size);

#endif
