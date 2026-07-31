package org.symbol.examples;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

import org.symbol.sdk.CryptoTypes.PrivateKey;
import org.symbol.sdk.symbol.KeyPair;

/**
 * Argument-parsing and file-loading helpers shared by the runnable example mains.
 */
final class ExamplesUtils {
	private ExamplesUtils() {
	}

	/** Extracts the value of a {@code --flag value} or {@code --flag=value} argument; last occurrence wins, {@code null} when absent. */
	static String parseFlag(final String[] args, final String flag) {
		String value = null;
		for (int i = 0; i < args.length; ++i) {
			if (flag.equals(args[i]) && i + 1 < args.length)
				value = args[++i];
			else if (args[i].startsWith(flag + "="))
				value = args[i].substring(flag.length() + 1);
		}

		return value;
	}

	/** Reads a UTF-8 file from disk. */
	static String readContents(final Path filepath) {
		try {
			return Files.readString(filepath);
		} catch (IOException ex) {
			throw new RuntimeException("failed to read " + filepath, ex);
		}
	}

	/** Reads a private key (hex, trimmed) from a file and wraps it in a Symbol {@link KeyPair}. */
	static KeyPair readPrivateKey(final Path filepath) {
		return new KeyPair(new PrivateKey(readContents(filepath).trim()));
	}
}
