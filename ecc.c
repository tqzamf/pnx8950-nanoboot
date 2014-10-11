#include <stdint.h>

static inline uint32_t ecc_correct(uint8_t *data, uint32_t ecc) {
	// repack ECC bits into a standard odd-parity hamming code and a
	// global parity bit stored with an 11-way repetition code. this
	// makes all further calculations a lot easier. it also shows that
	// standard NAND ECC is actually a rather dumb way of storing a
	// normal SECDED extended hamming code...
	uint32_t hamming_read = 0, parity_read = 0;
	for (uint32_t bit = 1; bit & 0x7ff; bit <<= 1) {
		parity_read |= ecc & bit;
		ecc >>= 1;
		hamming_read |= ecc & bit;
	}
	parity_read ^= hamming_read;
	
	// recalculate parities from the data actually read off the device.
	uint32_t rpar = 0xff, cpar = 0x07;
	for (uint32_t pos = 0; pos < 256; pos++) {
		uint8_t byte = data[pos];
		
		// calculate column parity. reuses intermediate results where
		// possible, and calculated each bit at the position where it
		// belongs in the final result to save a shift.
		uint32_t l2 = byte ^ (byte << 2);
		uint32_t l2r1 = l2 ^ (l2 >> 1);
		cpar ^= l2r1 & 4;
		uint32_t r4 = byte ^ (byte >> 4);
		uint32_t r4l1 = r4 ^ (r4 << 1);
		cpar ^= r4l1 & 2;
		uint32_t r4r2 = r4 ^ (r4 >> 2);
		cpar ^= r4r2 & 1;
		
		// calculate overall parity of the byte. the result ends up in
		// the lowest bit; we then use negation to mirror that bit into
		// all other bits of the word.
		uint32_t r4r2r1 = r4r2 ^ (r4r2 >> 1);
		uint32_t par = -(r4r2r1 & 1);
		
		// calculate row parity. the neat thing about the normal, packed
		// representation is that the parity of a byte should contribute
		// to exactly those bits of the final result which are set in
		// the position of the byte, ie. row parity can be calculated in
		// parallel for all bits.
		rpar ^= par & pos;
		// the column parity code above actually calculates the parity
		// over the wrong bits. we correct that here, and in doing so
		// calculate the overall parity as well. this is slightly smaller
		// than calculating the parity over the right bits, because then
		// we'd have to calculate the overall parity separately.
		cpar ^= par;
	}
	uint32_t hamming_calc = (rpar << 3) | (cpar & 0x7);
	uint32_t parity_calc = (cpar >> 3) & 0x7ff;
	
	if (hamming_calc == hamming_read && parity_read == parity_calc)
		// all bits except the unused ones agree, ie. there was no error.
		return 0;
	
	// TODO detect and correct single-bit errors
	
	// uncorrectable error
	return 2;
}
