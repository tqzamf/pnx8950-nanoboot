#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <err.h>
#include <stdlib.h>

#include "ecc.c"

static inline int affects(int errpos_byte, uint8_t errpos_bit) {
	int num_errors_lower, num_errors_upper;
	if (errpos_byte < 256) {
		num_errors_lower = 1;
		num_errors_upper = 0;
	} else if (errpos_byte < 512) {
		num_errors_lower = 0;
		num_errors_upper = 1;
	} else if (errpos_byte == 512 + 0 || errpos_byte == 512 + 1) {
		num_errors_lower = 1;
		num_errors_upper = 0;
	} else if (errpos_byte == 512 + 3 || errpos_byte == 512 + 6) {
		num_errors_lower = 0;
		num_errors_upper = 1;
	} else if (errpos_byte == 512 + 2 && errpos_bit != 0x01 && errpos_bit != 0x02) {
		num_errors_lower = 1;
		num_errors_upper = 0;
	} else if (errpos_byte == 512 + 7 && errpos_bit != 0x01 && errpos_bit != 0x02) {
		num_errors_lower = 0;
		num_errors_upper = 1;
	} else {
		// OOB bit not used by ECC, so no error expected
		num_errors_lower = 0;
		num_errors_upper = 0;
	}
	return num_errors_lower + (num_errors_upper << 8);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: ecctester path-to-nand-image\n");
		return -1;
	}
	
	srand(time(NULL));
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		err(1, "cannot open %s", argv[1]);

	uint8_t buffer[512 + 16];
	int page = 0;
	while (1) {
		int res = read(fd, buffer, 512 + 16);
		if (res == 0)
			break;
		if (res != 512 + 16)
			err(1, "cannot read page %04x + OOB", page);
		
		uint8_t *oob = buffer + 512;
		uint8_t ecc0, ecc1, ecc2;
		
		// test ECC calculation. the input is correct so there should be
		// no disagreement with calculated values.
		
		ecc0 = oob[0];
		ecc1 = oob[1];
		ecc2 = oob[2];
		uint32_t ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);
		uint32_t num_errors = ecc_correct(buffer + 0, ecc);
		if (num_errors)
			fprintf(stderr, "error: page %04xL has %d errors\n", page, num_errors);

		ecc0 = oob[3];
		ecc1 = oob[6];
		ecc2 = oob[7];
		ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);
		num_errors = ecc_correct(buffer + 256, ecc);
		if (num_errors)
			fprintf(stderr, "error: page %04xU has %d errors\n", page, num_errors);
		
		// test single-bit error correction. corrupt a bit and see if it gets
		// corrected properly.
		
		uint8_t errpos_bit = 1 << (page & 0x7);
		int errpos_byte = (page >> 3) % (512 + 16);
		fprintf(stderr, "page %04x: corrupting %03x bit %02x\n",
				page, errpos_byte, errpos_bit);
		int expect = affects(errpos_byte, errpos_bit);
		int num_errors_lower = expect & 0xff;
		int num_errors_upper = expect >> 8;
		uint8_t orig_byte = buffer[errpos_byte];
		buffer[errpos_byte] = orig_byte ^ errpos_bit;
		
		ecc0 = oob[0];
		ecc1 = oob[1];
		ecc2 = oob[2];
		ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);
		num_errors = ecc_correct(buffer + 0, ecc);
		if (num_errors != num_errors_lower)
			fprintf(stderr, "error: page %04xL has %d errors\n", page, num_errors);

		ecc0 = oob[3];
		ecc1 = oob[6];
		ecc2 = oob[7];
		ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);
		num_errors = ecc_correct(buffer + 256, ecc);
		if (num_errors != num_errors_upper)
			fprintf(stderr, "error: page %04xU has %d errors\n", page, num_errors);
		
		if (orig_byte != buffer[errpos_byte] && errpos_byte < 512)
			fprintf(stderr, "error: page %04x error in byte %03x bit %02x not corrected:"
					"%02x != %02x\n", page, errpos_byte, errpos_bit, orig_byte, buffer[errpos_byte]);
		buffer[errpos_byte] = orig_byte;
		
		// test double-bit error detection. corrupt two bits and see if it gets
		// detected properly.
		
		// random second bit. not exhaustive, but good enough.
		int random = rand();
		uint8_t errpos2_bit = 1 << (random & 0x7);
		int errpos2_byte = (random >> 3) % (512 + 16);
		fprintf(stderr, "page %04x: corrupting %03x bit %02x and %03x bit %02x\n",
				page, errpos_byte, errpos_bit, errpos2_byte, errpos2_bit);
		expect = affects(errpos_byte, errpos_bit);
		num_errors_lower = expect & 0xff;
		num_errors_upper = expect >> 8;
		expect = affects(errpos2_byte, errpos2_bit);
		num_errors_lower += expect & 0xff;
		num_errors_upper += expect >> 8;
		if (errpos2_bit == errpos_bit && errpos2_byte == errpos_byte)
			// flipping the same bit back and forth. not an effective test,
			// but don't misreport it as an error.
			num_errors_lower = num_errors_upper = 0;
		buffer[errpos_byte] = buffer[errpos_byte] ^ errpos_bit;
		buffer[errpos2_byte] = buffer[errpos2_byte] ^ errpos2_bit;
		
		ecc0 = oob[0];
		ecc1 = oob[1];
		ecc2 = oob[2];
		ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);
		num_errors = ecc_correct(buffer + 0, ecc);
		if (num_errors != num_errors_lower)
			fprintf(stderr, "error: page %04xL has %d errors\n", page, num_errors);

		ecc0 = oob[3];
		ecc1 = oob[6];
		ecc2 = oob[7];
		ecc = (ecc0 << 14) | (ecc1 << 6) | (ecc2 >> 2);
		num_errors = ecc_correct(buffer + 256, ecc);
		if (num_errors != num_errors_upper)
			fprintf(stderr, "error: page %04xU has %d errors\n", page, num_errors);
		
		page++;
	}

	return 0;
}
