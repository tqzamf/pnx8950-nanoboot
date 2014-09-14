import java.util.Arrays;
import java.util.List;

public class StoreInstruction extends BaseInstruction {
	private final Module module;
	private final int offset;
	private final int base;
	private final long[] values;

	public StoreInstruction(final Module module, final int offset,
			final long... values) {
		this.module = module;
		module.checkOffset(offset);
		this.offset = offset;
		base = module.getBase() + offset;
		this.values = values;
	}

	public StoreInstruction(final int base, final long... values) {
		module = null;
		offset = base;
		this.base = base;
		this.values = values;
	}

	@Override
	public String toString() {
		final StringBuilder buffer = new StringBuilder();
		if (module != null)
			buffer.append(module.getName()).append('.')
					.append(String.format("%03x", offset));
		else
			buffer.append(String.format("%08x", base));

		// if (values.length != 1)
		// buffer.append('[').append(values.length).append(']');
		buffer.append(" = ");
		if (values.length != 1) {
			buffer.append("{ ");
			for (final long val : values)
				buffer.append(String.format("%08x", val)).append(", ");
			buffer.append("}");
		} else
			buffer.append(String.format("%08x", values[0]));
		return buffer.toString();
	}

	@Override
	public List<BaseInstruction> toBaseInstructions() {
		return Arrays.<BaseInstruction> asList(this);
	}

	@Override
	public boolean canCombineWith(final BaseInstruction ins) {
		// can combine stores, but only if they are adjacent and consecutive.
		// also restrict to the same module to avoid confusion.
		if (ins instanceof StoreInstruction) {
			final StoreInstruction store = (StoreInstruction) ins;
			return store.module == module
					&& store.base == base + 4 * values.length;
		}
		return false;
	}

	@Override
	public BaseInstruction combineWith(final BaseInstruction ins) {
		final StoreInstruction store = (StoreInstruction) ins;
		final long[] newValues = new long[values.length + store.values.length];
		System.arraycopy(values, 0, newValues, 0, values.length);
		System.arraycopy(store.values, 0, newValues, values.length,
				store.values.length);

		if (module == null)
			return new StoreInstruction(base, newValues);
		else
			return new StoreInstruction(module, offset, newValues);
	}

	@Override
	public byte[] assemble() {
		if (values.length == 1)
			return assemble(Opcode.STORE, base, values[0]);

		final long[] newValues = new long[values.length + 2];
		newValues[0] = base;
		newValues[1] = values.length;
		System.arraycopy(values, 0, newValues, 2, values.length);
		return assemble(Opcode.FILL, newValues);
	}
}
