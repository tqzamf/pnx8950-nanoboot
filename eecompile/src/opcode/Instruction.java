package opcode;
import java.util.List;

public abstract class Instruction {
	public abstract List<BaseInstruction> toBaseInstructions();
}
