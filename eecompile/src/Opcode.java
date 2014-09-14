public enum Opcode {
	SLEEP(0x0f, 0x02), HALT(0x0f, 0x06), STORE(0x03, 0x00), FILL(0x03, 0x01);

	private final int mask;
	private final int bits;

	private Opcode(final int mask, final int bits) {
		this.mask = mask;
		this.bits = bits;
	}

	public long getWord(final long data) {
		if ((data & mask) != 0)
			throw new IllegalArgumentException("unaligned data: "
					+ String.format("%08x", data));
		if (data > 0xffffffffl || data < 0)
			throw new IllegalArgumentException("data out of range: "
					+ String.format("%08x", data));
		return data | bits;
	}
}
