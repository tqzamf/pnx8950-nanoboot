package output;
import java.io.File;
import java.io.IOException;

public abstract class Formatter {
	public abstract void write(byte[] data, File output) throws IOException;
}
