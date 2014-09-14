import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

public class BinaryFormatter extends Formatter {
	@Override
	public void write(final byte[] data, final File output) throws IOException {
		final BufferedOutputStream out = new BufferedOutputStream(
				new FileOutputStream(output));
		out.write(data);
		out.close();
	}
}
