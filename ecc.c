#include <stdint.h>

static inline uint32_t ecc_correct(uint8_t *data, uint32_t ecc) {
	// recalculate parities from the data actually read off the device.
	// because even ^ odd == (overall) parity, it is sufficient to calculate
	// half of the parity bits, and the overall parity.
	// the odd parity bits are more straight-forward to calculate than the
	// even ones, so we calculate the odd ones.
	uint32_t parity = 0x7ff;
	uint32_t pos = 256;
	do {
		pos--;
		uint8_t byte = data[pos];
		
		// calculate column parity. uses the lookup-table-in-immediate
		// approach because it is actually smaller than the equivalent
		// computation. also, because the lowest bit is completely
		// ignored for both cp1 and cp3, these two "lookup tables" are
		// folded into a single 2-bit lookup table, which is even more
		// efficient.
		//
		// covered bits:
		// bits  7 6 5 4  3 2 1 0
		//  cp1  x   x    x   x
		//  cp3  x x      x x
		//  cp5  x x x x
		//   rp  x x x x  x x x x
		//
		// bits  cp3cp1 cp5 rp    bits  cp3cp1 cp5 rp
		// 0000    0  0   0  0    1000    1  1   1  1
		// 0001    0  0   1  1    1001    1  1   0  0
		// 0010    0  1   1  1    1010    1  0   0  0
		// 0011    0  1   0  0    1011    1  0   1  1
		// 0100    1  0   1  1    1100    0  1   0  0
		// 0101    1  0   0  0    1101    0  1   1  1
		// 0110    1  1   0  0    1110    0  0   1  1
		// 0111    1  1   1  1    1111    0  0   0  0
		//
		// nibbles  fe dc ba 98  76 54 32 10
		// cp1cp3   00 01 10 11  11 10 01 00  1BE4
		// nibble   fedc  ba98   7654  3210
		// cp5      0110  1001   1001  0110   6996
		// rp       0110  1001   1001  0110   6996
		uint32_t temp = byte ^ (byte >> 4);
		uint32_t rp     = (0x6996 >> (temp & 15)) & 1;
		uint32_t cp5    = (0x6996 >> (byte >> 4)) & 1;
		uint32_t cp3cp1 = (0x1BE4 >> (temp & 14)) & 3;
		
		// accumulate column parity in the lower three bits.
		parity ^= (cp5 << 2) | cp3cp1;
		
		// accumulate row parity in bits 3..11 by XORing the byte's
		// overall parity into the right bits of the accumulator.
		// to do this in parallel, the row parity bit in the LSB is
		// first mirrored, and then the bits that should remain
		// unaffected are masked out. because each row bit should be
		// affected iff the corresponding bit in the byte's position is
		// set, this is a single efficient bitwise AND.
		// this also accumulates overall parity in the upper 16 bits by just
		// XORing in every byte into these bits.
		uint32_t par = -rp;
		uint32_t bits = (0x7ff << 16) | (pos << 3);
		parity ^= par & bits;
	} while (pos);
	// finish even parity calculation using even == odd ^ (overall) parity.
	//uint32_t overall = parity >> 16;
	parity ^= parity << 16;

	// repack ECC bits into a pair of standard hamming codes, with even
	// and odd parity respectively. this makes all calculations a lot
	// easier.
	// the odd bits can be combined directly with the calculated data; for
	// the even bits, it's faster to do that separately later on.
	uint32_t even_bits = 0;
	for (uint32_t bit = 1; bit < 0x800; bit <<= 1) {
		even_bits ^= ecc & bit;
		ecc >>= 1;
		parity ^= ecc & bit;
	}
	parity ^= even_bits << 16;
	
	if (parity == 0)
		// common case: all bits agree with the calculated ones. no
		// error to correct.
		return 0;
	
	uint32_t odd = parity & 0x7ff;
	uint32_t even = parity >> 16;
	if ((odd ^ even) == 0x7ff) {
		// the reference code checks that the even and odd parity bits are
		// exact complements, ie. they agree on a particular bit that is
		// faulty. another way of seeing it is that because of even ^ odd
		// == parity, this really just means that parity == 0x7ff, ie. the
		// overall parity is self-consistent but exactly the opposite of
		// what was read, indicating a single-bit error.
		// conveniently the odd parity syndrome gives the bit position that
		// is in error.
		uint32_t errpos_bit = 1 << (odd & 0x7);
		uint32_t errpos_byte = odd >> 3;
		data[errpos_byte] ^= errpos_bit;
		return 1;
	}
	
	if ((parity & (parity - 1)) == 0)
		// the even and odd parity disagree in a single bit, so it must
		// be this parity bit that is faulty. we do not have to correct
		// this bit because nobody cares about it any more, but the
		// error has to be reported anyway because another error would
		// make the block uncorrectable.
		// this single-bit-check is a variation of the classic Brian
		// Kernighan way of bitcounting. we actually use it to clear the
		// lowest bit, wherever it may be. the result will be zero if either
		// zero or one bits are set, but at this point we know there have to
		// be some bits that are set.
		return 1;
	
	// uncorrectable error, more than 2 bits differ
	return 2;
}
