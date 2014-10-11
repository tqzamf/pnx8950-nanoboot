#include <stdint.h>

static inline uint32_t ecc_correct(uint8_t *data, uint32_t ecc) {
	// repack ECC bits into a pair of standard hamming codes, with even
	// and odd parity respectively. this makes all further calculations
	// a bit easier.
	uint32_t hamming1_read = 0, hamming2_read = 0;
	for (uint32_t bit = 1; bit & 0x7ff; bit <<= 1) {
		hamming1_read |= ecc & bit;
		ecc >>= 1;
		hamming2_read |= ecc & bit;
	}
	
	// recalculate parities from the data actually read off the device.
	uint32_t parity = 0x7ff;
	for (uint32_t pos = 0; pos < 256; pos++) {
		uint8_t byte = data[pos];
		
		// calculate column parity. reuses intermediate results where
		// possible, and calculated each bit at the position where it
		// belongs in the final result to save a shift.
		uint32_t l2 = byte ^ (byte << 2);
		uint32_t l2r1 = l2 ^ (l2 >> 1);
		parity ^= l2r1 & 4;
		uint32_t r4 = byte ^ (byte >> 4);
		uint32_t r4l1 = r4 ^ (r4 << 1);
		parity ^= r4l1 & 2;
		uint32_t r4r2 = r4 ^ (r4 >> 2);
		parity ^= r4r2 & 1;
		
		// calculate overall parity of the byte. the result ends up in
		// the lowest bit; we then use negation to mirror that bit into
		// all other bits of the word.
		uint32_t r4r2r1 = r4r2 ^ (r4r2 >> 1);
		uint32_t par = -(r4r2r1 & 1);
		
		// the column parity code above actually calculates the parity
		// over the wrong bits. we correct that here, and in doing so
		// calculate both the overall parity and the row parity as well.
		// this is smaller than calculating the parity over the right
		// bits in the first place, because then we'd have to calculate
		// the other parities separately.
		// for row parity, we use the neat fact the in normal, packed
		// representation, the parity of a byte should contribute to
		// exactly those bits of the final result which are set in the
		// position of the byte, ie. row parity can be calculated in
		// parallel for all bits.
		// all in all, this means we have to XOR the byte's parity into:
		// - the column parity bits so they cover the right bits
		// - any row parity bits they should affect
		// - some higher-order bits, for overall parity
		uint32_t affected = 0x3ff807 | (pos << 3);
		parity ^= par & affected;
	}
	uint32_t hamming2_calc = parity & 0x7ff;
	// in the code above, we only calculate the parity for the odd parity
	// hamming code. we can now easily derive the even parity ones by
	// XORing them with the overall parity.
	uint32_t hamming1_calc = hamming2_calc ^ (parity >> 11);
	// calculate syndrome for both hamming codes.
	uint32_t syndrome1 = hamming1_calc ^ hamming1_read;
	uint32_t syndrome2 = hamming2_calc ^ hamming2_read;
	
	if (syndrome1 == 0 && syndrome2 == 0)
		// all bits agree with the calculated ones. no error.
		return 0;
	
	if ((syndrome1 ^ syndrome2) == 0x7ff) {
		// overall parity is self-consistent, but disagrees with what we
		// have calculated. this means there must have been a correctable
		// single-bit error (or a triple bit error that we are about to
		// mis-correct, but that's permissible).
		uint32_t errpos_bit = 1 << (syndrome2 & 0x7);
		uint32_t errpos_byte = syndrome2 >> 3;
		data[errpos_byte] ^= errpos_bit;
		return 1;
	}
	
	uint32_t bitcount = (syndrome1 << 11) | syndrome2;
	if ((bitcount & (bitcount - 1)) == 0)
		// overall parity is correct, except for a single bit. this means
		// there was an error in the ECC bits. an error in the ECC data
		// does not need correcting, but still has to be reported because
		// another error would make the block uncorrectable.
		// the single-bit-check is a variation of the classic Brian
		// Kernighan way of bitcounting. we actually use it to clear the
		// lowest bit, wherever it may be. if any other bit was set too,
		// then the result is necessarily nonzero.
		return 1;
	
	// uncorrectable error
	return 2;
}
