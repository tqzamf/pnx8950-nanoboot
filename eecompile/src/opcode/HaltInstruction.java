package opcode;

import java.util.Arrays;
import java.util.List;

public class HaltInstruction extends BaseInstruction {
	@Override
	public String toString() {
		return "halt";
	}

	@Override
	public List<BaseInstruction> toBaseInstructions() {
		return Arrays.<BaseInstruction> asList(this);
	}

	@Override
	public boolean canCombineWith(final BaseInstruction ins) {
		// can omit everything after "halt" to save space
		return true;
	}

	@Override
	public BaseInstruction combineWith(final BaseInstruction ins) {
		return this;
	}

	@Override
	public byte[] assemble() {
		return assemble(Opcode.HALT, 0);
	}
}
