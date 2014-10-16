package core;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.ParseException;

public abstract class Parser {
	public void parse(final File file) throws IOException, ParseException {
		parse(new FileInputStream(file));
	}

	public void parse(final InputStream file) throws IOException,
			ParseException {
		final BufferedReader in = new BufferedReader(new InputStreamReader(
				file, "utf8"));
		while (true) {
			final String line = in.readLine();
			if (line == null)
				break;
			final String l = line.replaceFirst("#.*$", "")
					.replaceAll("\\s+", " ").trim();
			if (!l.isEmpty())
				parseLine(l);
		}
		in.close();
	}

	protected abstract void parseLine(String line) throws ParseException;
}
