package core;

import java.util.HashMap;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ModuleParser extends Parser {
	private static final Pattern ADDR_RANGE = Pattern
			.compile("`([0-9a-fA-F]{6})-([0-9a-fA-F]{6})`");
	private static final Pattern MOD_NAME = Pattern
			.compile("`([0-9a-zA-Z-]*)`");

	private final Map<String, Module> modules = new HashMap<String, Module>();

	@Override
	protected void parseLine(final String line) {
		final String[] fields = line.split("\\|");
		final String addrRange = fields[1];
		final String modName = fields[4];
		final Matcher m = ADDR_RANGE.matcher(addrRange);
		final Matcher n = MOD_NAME.matcher(modName);
		if (m.find() && n.find()) {
			final int base = Integer.valueOf(m.group(1), 16);
			final int end = Integer.valueOf(m.group(2), 16);
			final String name = n.group(1).toLowerCase();
			modules.put(name, new Module(name, base, end));
		}
	}

	public Module getModule(final String name) {
		if (!modules.containsKey(name))
			throw new NoSuchElementException(name);
		return modules.get(name);
	}
}
