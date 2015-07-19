#ifndef _HEADER_H
#define _HEADER_H

#define IMAGE_HEADER_BASE DRAM_BASE
#define IMAGE_HEADER_SIZE 0x200
#define IMAGE_HEADER_SIGNATURE 0x57434530
struct __attribute__((packed, aligned(4))) image_header {
	uint32_t signature1;
	uint32_t length;
	uint32_t signature2;
	uint32_t loadaddr;
};

static inline int is_header(uint8_t *base) {
	struct image_header * header = (void *) base;
	return header->signature1 == IMAGE_HEADER_SIGNATURE
			&& header->signature2 == IMAGE_HEADER_SIGNATURE;
}

#endif
