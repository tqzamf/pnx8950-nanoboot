public class Module {
	private static final int MMIO_BASE = 0x1be00000;

	private final String name;
	private final int base;
	private final int size;
	private final int end;

	public Module(final String name, final int base, final int end) {
		this.name = name;
		this.base = base;
		this.end = end;
		size = end - base + 1;
	}

	public String getName() {
		return name;
	}

	public int getBase() {
		return base + MMIO_BASE;
	}

	public int getSize() {
		return size;
	}

	public int getEnd() {
		return end;
	}

	public boolean isValid(final int offset) {
		return offset >= 0 && offset <= size;
	}

	public void checkOffset(final int offset) {
		if (!isValid(offset))
			throw new IndexOutOfBoundsException("0 <= " + offset + " <= "
					+ size);
	}
}
