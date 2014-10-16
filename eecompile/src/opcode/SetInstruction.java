package opcode;

import java.util.ArrayList;
import java.util.List;

import core.Module;


public class SetInstruction extends Instruction {
	private final Module module;
	private final int offset;
	private final List<Long> values;
	private final int base;

	public SetInstruction(final Module module, final int offset,
			final String values) {
		this.module = module;
		module.checkOffset(offset);
		this.offset = offset;
		base = module.getBase() + offset;
		this.values = new ArrayList<Long>();
		for (final String val : values.split(" then "))
			this.values.add(Long.parseLong(val, 16));
	}

	public SetInstruction(final int base, final String values) {
		this.base = base;
		module = null;
		offset = base;
		this.values = new ArrayList<Long>();
		for (final String val : values.split(" then "))
			this.values.add(Long.parseLong(val, 16));
	}

	@Override
	public String toString() {
		final StringBuilder buffer = new StringBuilder();
		if (module != null)
			buffer.append(module.getName()).append('.')
					.append(String.format("%03x", offset));
		else
			buffer.append(String.format("%08x", base));
		buffer.append(" = ");
		for (int i = 0; i < values.size(); i++) {
			if (i > 0)
				buffer.append(" then ");
			buffer.append(String.format("%08x", values.get(i)));
		}
		return buffer.toString();
	}

	@Override
	public List<BaseInstruction> toBaseInstructions() {
		final ArrayList<BaseInstruction> instructions = new ArrayList<BaseInstruction>();
		for (final long value : values) {
			final StoreInstruction ins;
			if (module == null)
				ins = new StoreInstruction(base, value);
			else
				ins = new StoreInstruction(module, offset, value);
			instructions.add(ins);
		}
		return instructions;
	}
}
