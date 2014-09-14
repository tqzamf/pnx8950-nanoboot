import java.io.FileNotFoundException;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ScriptParser extends Parser {
	private static final Pattern SLEEP_PATTERN = Pattern
			.compile("sleep ([0-9]+) ?((?:[mu?]s)?)");
	private static final Pattern LOAD_PATTERN = Pattern
			.compile("load ([0-9A-Fa-f]{8}) (\"[^\"]+\"|'[^']+')");
	private static final Pattern SET_PATTERN_1 = Pattern
			.compile("([a-zA-Z][0-9a-zA-Z-]+)\\.([0-9A-Fa-f]{3,4}) = ([0-9A-Fa-f]{1,8}(?: then [0-9A-Fa-f]{1,8})*)");
	private static final Pattern SET_PATTERN_2 = Pattern
			.compile("([0-9A-Fa-f]{8}) = ([0-9A-Fa-f]{1,8}(?: then [0-9A-Fa-f]{1,8})*)");

	private final List<Instruction> instructions = new ArrayList<Instruction>();
	private final ModuleParser modules;

	public ScriptParser(final ModuleParser modules) {
		this.modules = modules;
	}

	@Override
	protected void parseLine(final String line) throws ParseException {
		instructions.add(parseInstr(line));
	}

	public Instruction parseInstr(final String line) throws ParseException {
		if (line.equalsIgnoreCase("halt"))
			return new HaltInstruction();

		final Matcher m1 = SLEEP_PATTERN.matcher(line);
		if (m1.matches()) {
			final int timeout = Integer.parseInt(m1.group(1), 10);
			final String unit = m1.group(2);
			return new SleepInstruction(timeout, unit);
		}

		final Matcher m2 = SET_PATTERN_1.matcher(line);
		if (m2.matches()) {
			final String module = m2.group(1);
			final int offset = Integer.parseInt(m2.group(2), 16);
			final String values = m2.group(3);
			return new SetInstruction(modules.getModule(module), offset, values);
		}

		final Matcher m3 = SET_PATTERN_2.matcher(line);
		if (m3.matches()) {
			final int base = Integer.parseInt(m3.group(1), 16);
			final String values = m3.group(2);
			return new SetInstruction(base, values);
		}

		final Matcher m4 = LOAD_PATTERN.matcher(line);
		if (m4.matches()) {
			final int base = Integer.parseInt(m4.group(1), 16);
			final String file = m4.group(2).replaceFirst("^[\"']", "")
					.replaceFirst("[\"']$", "");
			try {
				return new LoadInstruction(base, file);
			} catch (final FileNotFoundException e) {
				throw new ParseException(line + ": not found", m4.start(2));
			}
		}

		throw new ParseException(line, 0);
	}

	@Override
	public String toString() {
		final StringBuilder buffer = new StringBuilder();
		for (final Instruction instr : instructions)
			buffer.append(instr.toString()).append('\n');
		return buffer.toString();
	}

	public List<Instruction> getInstructions() {
		return instructions;
	}

	public List<BaseInstruction> toBaseInstructions() {
		final List<BaseInstruction> res = new ArrayList<BaseInstruction>();
		for (final Instruction ins : instructions)
			res.addAll(ins.toBaseInstructions());
		return res;
	}
}
