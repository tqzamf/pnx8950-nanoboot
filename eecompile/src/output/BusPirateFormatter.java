package output;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

public class BusPirateFormatter extends Formatter {
	@Override
	public void write(final byte[] data, final File output) throws IOException {
		final PrintWriter out = new PrintWriter(new OutputStreamWriter(
				new FileOutputStream(output), "utf8"));
		// need to offset everything by 1 byte. also, the first byte is
		// irrelevant, so we can just set it to anything we want.
		out.printf("[0xa0 0x00 0x2a");
		int i = 1;
		for (final int b : data) {
			// next address header. generate a new start condition and send the
			// next address.
			if (i % 16 == 0)
				out.printf("[0xa%1x 0x%02x", i >> 8 << 1, i & 255);
			out.printf(" 0x%02x", b & 255);
			// send a stop bit so that the chip starts writing. a re-start
			// aborts the write.
			if (i++ % 16 == 15)
				out.printf("]\n");
		}
		if (i % 16 != 0)
			out.printf("]\n");
		out.close();
	}
}
