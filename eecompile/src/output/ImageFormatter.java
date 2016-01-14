package output;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class ImageFormatter extends Formatter {
	private static final byte[] DUMMY_BYTE = { 0x2a };
	
	@Override
	public void write(final byte[] data, final File output) throws IOException {
		final BufferedOutputStream out = new BufferedOutputStream(
				new FileOutputStream(output));
		// if the chip isn't write-protected, the first byte is overwritten with
		// 0x00 by the boot module trying to send a 16-bit address. setting it
		// to some non-zero value allows checking whether write-protection works
		// by reading it back after boot.
		// the actual bootscript starts at address 1, ie. everything is offset
		// by 1 byte.
		out.write(DUMMY_BYTE);
		out.write(data);
		out.close();
	}
}
