import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

public class HexFormatter extends Formatter {
	@Override
	public void write(final byte[] data, final File output) throws IOException {
		final PrintWriter out = new PrintWriter(new OutputStreamWriter(
				new FileOutputStream(output), "utf8"));
		int i = 0;
		for (final int b : data) {
			if (i % 16 == 0)
				out.print(" ");
			out.printf(" %02x", b & 255);
			if (i++ % 16 == 15)
				out.println();
		}
		out.close();
	}
}
