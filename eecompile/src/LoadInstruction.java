import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class LoadInstruction extends Instruction {
	private final int base;
	private final File file;

	public LoadInstruction(final int base, final String file)
			throws FileNotFoundException {
		this.base = base;
		this.file = new File(file);
		if (!this.file.exists())
			throw new FileNotFoundException(file);
	}

	private ArrayList<Long> load() throws IOException {
		final DataInputStream in = new DataInputStream(
				new FileInputStream(file));
		final ArrayList<Long> values = new ArrayList<Long>();
		while (in.available() > 0) {
			final int word = in.readInt();
			values.add(word & 0xffffffffl);
		}
		in.close();
		return values;
	}

	@Override
	public String toString() {
		return "load " + String.format("%08x", base) + " '" + file.getName()
				+ "'";
	}

	@Override
	public List<BaseInstruction> toBaseInstructions() {
		ArrayList<Long> values;
		try {
			values = load();
		} catch (final IOException e) {
			throw new RuntimeException("cannot read " + file.getAbsolutePath(),
					e);
		}
		final ArrayList<BaseInstruction> instructions = new ArrayList<BaseInstruction>();
		int i = 0;
		for (final long value : values) {
			instructions.add(new StoreInstruction(base + i, value));
			i += 4;
		}
		return instructions;
	}
}
