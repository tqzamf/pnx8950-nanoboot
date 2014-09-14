import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;

public class Script {
	private final List<BaseInstruction> instructions = new ArrayList<BaseInstruction>();

	@Override
	public String toString() {
		final StringBuilder buffer = new StringBuilder();
		for (final Instruction instr : instructions)
			buffer.append(instr.toString()).append('\n');
		return buffer.toString();
	}

	public void add(final BaseInstruction instr) {
		instructions.add(instr);
	}

	public void add(final List<BaseInstruction> instr) {
		instructions.addAll(instr);
	}

	public Script optimize() {
		final Script opt = new Script();
		BaseInstruction prevIns = null;
		for (final BaseInstruction ins : instructions)
			if (prevIns == null || !prevIns.canCombineWith(ins)) {
				if (prevIns != null)
					opt.add(prevIns);
				prevIns = ins;
			} else
				prevIns = prevIns.combineWith(ins);

		final HaltInstruction halt = new HaltInstruction();
		if (!prevIns.canCombineWith(halt)) {
			opt.add(prevIns);
			opt.add(halt);
		} else
			opt.add(prevIns.combineWith(halt));
		return opt;
	}

	public byte[] compile() {
		final ByteArrayOutputStream buffer = new ByteArrayOutputStream();
		for (final BaseInstruction ins : instructions) {
			final byte[] bytecode = ins.assemble();
			buffer.write(bytecode, 0, bytecode.length);
		}
		return buffer.toByteArray();
	}
}
