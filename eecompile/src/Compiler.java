import java.io.File;
import java.io.IOException;
import java.text.ParseException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

public class Compiler {
	private final boolean optimize;
	private final ModuleParser modules;
	private final Script script;

	public Compiler(final boolean optimize, final File modpath)
			throws IOException, ParseException {
		this.optimize = optimize;
		modules = new ModuleParser();
		if (modpath != null)
			modules.parse(modpath);
		else
			modules.parse(Compiler.class
					.getResourceAsStream("modules.markdown"));
		script = new Script();
	}

	public void load(final File file) throws IOException, ParseException {
		final ScriptParser parser = new ScriptParser(modules);
		parser.parse(file);
		script.add(parser.toBaseInstructions());
	}

	public byte[] compile() {
		Script output;
		if (optimize)
			output = script.optimize();
		else
			output = script;
		return output.compile();
	}

	public static void main(final String... argv) throws IOException,
			ParseException {
		final Iterator<String> args = Arrays.asList(argv).iterator();
		boolean optimize = false;
		final List<File> files = new ArrayList<File>();
		File modpath = null;
		File output = null;
		Formatter formatter = new HexFormatter();
		boolean verbose = false;
		while (args.hasNext()) {
			final String arg = args.next();
			if (arg.equals("-v"))
				verbose = true;
			else if (arg.equals("-m"))
				modpath = new File(args.next());
			else if (arg.startsWith("-O")) {
				int level;
				if (arg.equals("-O"))
					level = Integer.parseInt(args.next());
				else
					level = Integer.parseInt(arg.substring(2));

				if (level == 0)
					optimize = false;
				else if (level == 1)
					optimize = true;
				else
					usage();
			} else if (arg.equals("-o")) {
				if (output != null)
					usage();
				output = new File(args.next());
			} else if (arg.equals("-f")) {
				final String format = args.next();
				if (format.equalsIgnoreCase("bin")
						|| format.equalsIgnoreCase("binary")
						|| format.equalsIgnoreCase("raw"))
					formatter = new BinaryFormatter();
				else if (format.equalsIgnoreCase("hex"))
					formatter = new HexFormatter();
				else if (format.equalsIgnoreCase("bp"))
					formatter = new BusPirateFormatter();
				else
					usage();
			} else if (arg.startsWith("-"))
				usage();
			else
				files.add(new File(arg));
		}
		if (output == null)
			usage();

		final Compiler c = new Compiler(optimize, modpath);
		for (final File file : files) {
			if (verbose)
				System.out.println("reading " + file.getName());
			c.load(file);
		}
		final byte[] data = c.compile();
		formatter.write(data, output);
		if (verbose)
			System.out.println("wrote " + data.length + " bytes to " + output);
	}

	private static void usage() {
		System.err
				.println("usage: eecompile [options] -o output.hex input.ee...");
		System.err
				.println("  -m path    set path to modules.markdown [builtin]");
		System.err.println("  -O level   enable optimization [0]");
		System.err.println("     0       don't optimize");
		System.err.println("     1       don't optimize");
		System.err.println("  -f format  set output format [hex]");
		System.err.println("     hex     hex pairs, space-separated");
		System.err.println("     bp      bus pirate commands");
		System.err.println("     raw     raw binary");
		System.err.println("  -v         verbose");
		System.exit(1);
	}

}
