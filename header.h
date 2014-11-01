#ifndef _HEADER_H
#define _HEADER_H

#define IMAGE_LOAD_ADDR     (DRAM_BASE | 0x00100000)
#define IMAGE_HEADER_SIZE 0x200
#define IMAGE_HEADER_SIGNATURE 0x57434530
#define IMAGE_HEADER_BASE ((void *) (IMAGE_LOAD_ADDR - IMAGE_HEADER_SIZE))
#define HEADER ((struct image_header *) IMAGE_HEADER_BASE)
struct __attribute__((packed)) image_header {
	uint32_t signature1;
	uint32_t length;
	uint32_t signature2;
	uint32_t entrypoint;
};

static inline int is_header(uint8_t *base) {
	struct image_header * header = (void *) base;
	return header->signature1 == IMAGE_HEADER_SIGNATURE
			&& header->signature2 == IMAGE_HEADER_SIGNATURE;
}
#define expect_header(base, header) \
		((void *) (base) == (void *) (header))

#endif
