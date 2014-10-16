package opcode;

import java.util.Arrays;
import java.util.List;

public class SleepInstruction extends BaseInstruction {
	private static final int FREQ = 27000000;

	private final int timeout;

	public SleepInstruction(final int timeout, final String unit) {
		if (unit == null || unit.isEmpty())
			this.timeout = timeout;
		else if (unit.equals("us"))
			this.timeout = timeout * FREQ;
		else if (unit.equals("ms"))
			this.timeout = timeout * FREQ / 1000;
		else if (unit.equals("s"))
			this.timeout = timeout * FREQ / 1000 / 1000;
		else
			throw new IllegalArgumentException("unknown time unit " + unit);
	}

	@Override
	public String toString() {
		return "sleep " + timeout + " # " + timeout * 1000000 / FREQ + "us";
	}

	@Override
	public List<BaseInstruction> toBaseInstructions() {
		return Arrays.<BaseInstruction> asList(this);
	}

	@Override
	public boolean canCombineWith(final BaseInstruction ins) {
		// sleep timeouts can be added up, unless there is an overflow
		if (ins instanceof SleepInstruction) {
			final SleepInstruction sleep = (SleepInstruction) ins;
			return timeout + sleep.timeout <= 0x0fffffff;
		}
		// sleep before halt can be omitted
		if (ins instanceof HaltInstruction)
			return true;
		return false;
	}

	@Override
	public BaseInstruction combineWith(final BaseInstruction ins) {
		// sleep timeouts can be added up, unless there is an overflow
		if (ins instanceof SleepInstruction) {
			final SleepInstruction sleep = (SleepInstruction) ins;
			return new SleepInstruction(timeout + sleep.timeout, null);
		}
		// sleep before halt can be omitted
		if (ins instanceof HaltInstruction)
			return ins;

		throw new IllegalArgumentException("cannot combine with " + ins);
	}

	@Override
	public byte[] assemble() {
		return assemble(Opcode.SLEEP, timeout << 4);
	}
}
