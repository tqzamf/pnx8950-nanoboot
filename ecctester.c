#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <err.h>
#include <setjmp.h>
#include <signal.h>

#include "ecc.c"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: ecctester path-to-nand-image >output\n");
		return -1;
	}
	
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		err(1, "cannot open %s", argv[1]);

	uint32_t buffer[512/4 + 16/4];
	int page = 0;
	while (1) {
		int res = read(fd, buffer, 512 + 16);
		if (res == 0)
			break;
		if (res != 512 + 16)
			err(1, "cannot read page %04x + OOB", page);
		
		uint8_t *oob = ((uint8_t *) buffer) + 512;
		
		uint8_t ecc0, ecc1, ecc2;
		ecc0 = oob[0];
		ecc1 = oob[1];
		ecc2 = oob[2];
		uint32_t ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);
		uint32_t num_errors = ecc_correct(((uint8_t *) buffer) + 0, ecc);
		if (num_errors)
			fprintf(stderr, "page %04xL has %d errors\n", page, num_errors);

		ecc0 = oob[3];
		ecc1 = oob[6];
		ecc2 = oob[7];
		ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);
		num_errors = ecc_correct(((uint8_t *) buffer) + 256, ecc);
		if (num_errors)
			fprintf(stderr, "page %04xU has %d errors\n", page, num_errors);
		
		write(1, buffer, 512);
		page++;
	}

	return 0;
}
