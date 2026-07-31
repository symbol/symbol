package org.symbol.examples;

import java.io.IOException;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.List;
import java.util.stream.Stream;

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

	/**
	 * Finds bundled resource files named {@code <prefix>*<suffix>} by probing every directory classpath root; the singular
	 * {@code getResource("")} / {@code getResource("/")} forms cannot be used — they return the class's package directory /
	 * whichever root happens to be first (typically the classes directory), not the resources root. Requiring a regular file
	 * with both affixes keeps a classes root from false-positiving the probe, and an unreadable root is skipped like a
	 * non-file URL rather than aborting the scan.
	 *
	 * @param prefix Required filename prefix.
	 * @param suffix Required filename suffix.
	 * @return Matching files in filename order, from the first classpath root containing any.
	 */
	static List<Path> findBundledFiles(final String prefix, final String suffix) {
		final Enumeration<URL> roots;
		try {
			roots = ExamplesUtils.class.getClassLoader().getResources("");
		} catch (final IOException ex) {
			throw new IllegalStateException("cannot enumerate classpath roots", ex);
		}

		while (roots.hasMoreElements()) {
			final URL root = roots.nextElement();
			if (!"file".equalsIgnoreCase(root.getProtocol()))
				continue;

			try {
				final List<Path> files = listFiles(Paths.get(root.toURI()), prefix, suffix);
				if (!files.isEmpty())
					return files;
			} catch (final IOException | URISyntaxException ex) {
				// an unreadable root must not abort the probe of the remaining roots
			}
		}

		throw new IllegalStateException("no classpath root contains " + prefix + "*" + suffix + " resources");
	}

	private static List<Path> listFiles(final Path root, final String prefix, final String suffix) throws IOException {
		try (final Stream<Path> stream = Files.list(root)) {
			return stream
					.filter(Files::isRegularFile)
					.filter(p -> p.getFileName().toString().startsWith(prefix) && p.getFileName().toString().endsWith(suffix))
					.sorted(Comparator.comparing(p -> p.getFileName().toString()))
					.toList();
		}
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
