package org.symbol.examples;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

import org.symbol.sdk.CryptoTypes.PrivateKey;
import org.symbol.sdk.symbol.KeyPair;

/**
 * File-loading helpers shared by the runnable example mains.
 */
final class ExamplesUtils {
	private ExamplesUtils() {
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
