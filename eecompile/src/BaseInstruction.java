public abstract class BaseInstruction extends Instruction {
	public abstract boolean canCombineWith(final BaseInstruction ins);

	public abstract BaseInstruction combineWith(final BaseInstruction ins);

	public abstract byte[] assemble();

	protected byte[] assemble(final Opcode opcode, final long... data) {
		data[0] = opcode.getWord(data[0]);
		final byte[] bytes = new byte[4 * data.length];
		int i = 0;
		for (final long w : data) {
			bytes[i++] = (byte) w;
			bytes[i++] = (byte) (w >> 8);
			bytes[i++] = (byte) (w >> 16);
			bytes[i++] = (byte) (w >> 24);
		}
		return bytes;
	}
}