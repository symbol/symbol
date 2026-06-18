package org.symbol.sdk;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.text.Normalizer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.crypto.Mac;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;

/**
 * BIP-32 (HMAC-SHA512 chain derivation) and BIP-39 (mnemonic ↔ seed) implementation. Only the English wordlist is bundled; other languages
 * can be added by dropping additional resources beside {@code bip39-english.txt}.
 */
public final class Bip32 {
	private static final int PRIVATE_KEY_SIZE = 32;
	private static final int CHAIN_CODE_SIZE = 32;
	private static final int PBKDF2_ITERATIONS = 2048;
	private static final int SEED_BYTES = 64;

	/** {@link SecureRandom} is thread-safe; one shared instance avoids per-call construction cost. */
	private static final SecureRandom SECURE_RANDOM = new SecureRandom();

	private final byte[] rootHmacKey;
	private final String mnemonicLanguage;

	/**
	 * Creates a BIP-32 root-node factory using the default {@code ed25519} curve and the English wordlist.
	 */
	public Bip32() {
		this("ed25519", "english");
	}

	/**
	 * Creates a BIP-32 root-node factory using {@code curveName} and the English wordlist.
	 *
	 * @param curveName Elliptic curve to use (e.g. {@code ed25519} or {@code ed25519-keccak}).
	 */
	public Bip32(final String curveName) {
		this(curveName, "english");
	}

	/**
	 * Creates a BIP-32 root-node factory.
	 *
	 * @param curveName Elliptic curve to use.
	 * @param mnemonicLanguage Language of constructed mnemonics.
	 */
	public Bip32(final String curveName, final String mnemonicLanguage) {
		this.rootHmacKey = (curveName + " seed").getBytes(StandardCharsets.UTF_8);
		this.mnemonicLanguage = mnemonicLanguage;
	}

	/**
	 * Creates a BIP-32 root node from a seed.
	 *
	 * @param seed BIP-32 seed (typically 64 bytes from BIP-39 mnemonic + password).
	 * @return BIP-32 root node.
	 */
	public Bip32Node fromSeed(final byte[] seed) {
		return new Bip32Node(rootHmacKey, seed);
	}

	/**
	 * Creates a BIP-32 root node from a BIP-39 mnemonic and password.
	 *
	 * @param mnemonic BIP-39 mnemonic phrase.
	 * @param password BIP-39 mnemonic password (use empty string when there is no password).
	 * @return BIP-32 root node.
	 */
	public Bip32Node fromMnemonic(final String mnemonic, final String password) {
		final Wordlist wordlist = Wordlist.forLanguage(mnemonicLanguage);
		// validate the mnemonic against the wordlist first so a typo is reported early
		final String normalizedMnemonic = Normalizer.normalize(mnemonic, Normalizer.Form.NFKD);
		final String[] words = normalizedMnemonic.split("\\s+");
		Mnemonic.assertValid(words, wordlist.indexByWord);
		// reject non-canonical whitespace: the seed derives from the exact phrase bytes, so a
		// double-spaced phrase would silently yield different keys than its canonical form
		if (!normalizedMnemonic.equals(String.join(" ", words)))
			throw new IllegalArgumentException("mnemonic whitespace is not canonical (single spaces, no leading/trailing)");

		final byte[] seed = Mnemonic.toSeed(normalizedMnemonic, password);
		return fromSeed(seed);
	}

	/**
	 * Creates a random BIP-39 mnemonic with the default 32-byte entropy length (24 words).
	 *
	 * @return Random BIP-39 mnemonic phrase.
	 */
	public String random() {
		return random(32);
	}

	/**
	 * Creates a random BIP-39 mnemonic.
	 *
	 * @param seedLength Entropy length in bytes; must be one of {16, 20, 24, 28, 32}.
	 * @return Random mnemonic phrase.
	 */
	public String random(final int seedLength) {
		final int ent = seedLength * 8;
		if (128 > ent || 256 < ent || 0 != ent % 32)
			throw new IllegalArgumentException("Invalid Argument: ENT must be a multiple of 32, between 128 and 256 bits");

		final byte[] entropy = new byte[seedLength];
		SECURE_RANDOM.nextBytes(entropy);
		final Wordlist wordlist = Wordlist.forLanguage(mnemonicLanguage);
		return Mnemonic.fromEntropy(entropy, wordlist.words);
	}

	/** Representation of a BIP-32 node. */
	public static final class Bip32Node {
		/** Private key associated with this node. */
		public final CryptoTypes.PrivateKey privateKey;
		/** Chain code associated with this node. */
		public final byte[] chainCode;

		Bip32Node(final byte[] hmacKey, final byte[] data) {
			final byte[] hmacResult = hmacSha512(hmacKey, data);
			final byte[] privBytes = new byte[PRIVATE_KEY_SIZE];
			System.arraycopy(hmacResult, 0, privBytes, 0, PRIVATE_KEY_SIZE);
			this.privateKey = new CryptoTypes.PrivateKey(privBytes);
			final byte[] cc = new byte[CHAIN_CODE_SIZE];
			System.arraycopy(hmacResult, PRIVATE_KEY_SIZE, cc, 0, CHAIN_CODE_SIZE);
			this.chainCode = cc;
		}

		/**
		 * Derives a direct child node with the given identifier.
		 *
		 * @param identifier Child identifier.
		 * @return Child BIP-32 node.
		 */
		public Bip32Node deriveOne(final int identifier) {
			final byte[] childData = new byte[1 + PRIVATE_KEY_SIZE + 4];
			childData[0] = 0;
			childData[childData.length - 4] = (byte) 0x80;

			for (int i = 0; 4 > i; ++i) {
				final int idx = childData.length - 1 - i;
				childData[idx] = (byte) ((childData[idx] & 0xFF) | ((identifier >> (8 * i)) & 0xFF));
			}

			System.arraycopy(privateKey.bytes(), 0, childData, 1, PRIVATE_KEY_SIZE);
			return new Bip32Node(chainCode, childData);
		}

		/**
		 * Derives a descendent node along the given path.
		 *
		 * @param path BIP-32 path.
		 * @return Descendent node.
		 */
		public Bip32Node derivePath(final int[] path) {
			Bip32Node next = this;
			for (int identifier : path)
				next = next.deriveOne(identifier);
			return next;
		}
	}

	private static byte[] hmacSha512(final byte[] key, final byte[] data) {
		try {
			final Mac mac = Mac.getInstance("HmacSHA512");
			mac.init(new SecretKeySpec(key, "HmacSHA512"));
			return mac.doFinal(data);
		} catch (java.security.GeneralSecurityException ex) {
			throw new CryptoException(ex);
		}
	}

	/** BIP-39 mnemonic ↔ seed and ↔ entropy operations. */
	private static final class Mnemonic {
		private Mnemonic() {
		}

		static byte[] toSeed(final String normalizedMnemonic, final String password) {
			final String normalizedPassword = Normalizer.normalize(null == password ? "" : password, Normalizer.Form.NFKD);
			final char[] passwordChars = normalizedMnemonic.toCharArray();
			final byte[] saltBytes = ("mnemonic" + normalizedPassword).getBytes(StandardCharsets.UTF_8);
			try {
				final SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA512");
				final KeySpec spec = new PBEKeySpec(passwordChars, saltBytes, PBKDF2_ITERATIONS, SEED_BYTES * 8);
				return factory.generateSecret(spec).getEncoded();
			} catch (NoSuchAlgorithmException | java.security.spec.InvalidKeySpecException ex) {
				throw new CryptoException(ex);
			}
		}

		static String fromEntropy(final byte[] entropy, final List<String> wordlist) {
			final int ent = entropy.length * 8;
			final int cs = ent / 32;

			MessageDigest sha256;
			try {
				sha256 = MessageDigest.getInstance("SHA-256");
			} catch (NoSuchAlgorithmException ex) {
				throw new CryptoException(ex);
			}
			final byte[] hash = sha256.digest(entropy);

			// build the bit string ENT||checksum
			final StringBuilder bits = new StringBuilder(ent + cs);
			for (byte b : entropy)
				appendBits(bits, b & 0xFF, 8);
			// append cs bits from the start of hash
			for (int i = 0; i < cs; ++i) {
				final int byteIdx = i / 8;
				final int bitIdx = 7 - (i % 8);
				bits.append((hash[byteIdx] >> bitIdx) & 1);
			}

			final int totalBits = bits.length();
			final List<String> words = new ArrayList<>(totalBits / 11);
			for (int i = 0; i < totalBits; i += 11) {
				final int idx = Integer.parseInt(bits.substring(i, i + 11), 2);
				words.add(wordlist.get(idx));
			}
			return String.join(" ", words);
		}

		static void assertValid(final String[] words, final Map<String, Integer> indexByWord) {
			if (12 != words.length && 15 != words.length && 18 != words.length && 21 != words.length && 24 != words.length)
				throw new IllegalArgumentException("Mnemonic string is invalid: bad word count " + words.length);

			final StringBuilder bits = new StringBuilder(words.length * 11);
			for (String w : words) {
				final Integer idx = indexByWord.get(w);
				if (null == idx)
					throw new IllegalArgumentException("Mnemonic string is invalid: word not in wordlist: " + w);

				appendBits(bits, idx, 11);
			}

			final int totalBits = words.length * 11;
			final int cs = totalBits / 33;
			final int ent = totalBits - cs;
			final byte[] entropy = new byte[ent / 8];
			for (int i = 0; i < entropy.length; ++i)
				entropy[i] = (byte) Integer.parseInt(bits.substring(i * 8, i * 8 + 8), 2);

			MessageDigest sha256;
			try {
				sha256 = MessageDigest.getInstance("SHA-256");
			} catch (NoSuchAlgorithmException ex) {
				throw new CryptoException(ex);
			}
			final byte[] hash = sha256.digest(entropy);
			for (int i = 0; i < cs; ++i) {
				final int expected = (hash[i / 8] >> (7 - (i % 8))) & 1;
				final int actual = bits.charAt(ent + i) - '0';
				if (expected != actual)
					throw new IllegalArgumentException("Mnemonic string is invalid: checksum mismatch");
			}
		}

		private static void appendBits(final StringBuilder out, final int value, final int width) {
			final String s = Integer.toBinaryString(value);
			for (int i = s.length(); i < width; ++i)
				out.append('0');
			out.append(s);
		}
	}

	/**
	 * Loads BIP-39 wordlists from classpath resources. The bundled {@code bip39-<language>.txt} files are the official specification
	 * wordlists from {@code bitcoin/bips} (bip-0039), vendored verbatim like the catbuffer schemas. TODO: check for a lightweight library
	 * that can do this instead of bundling our own copy of the wordlists and loader code.
	 */
	private static final class Wordlist {
		// computeIfAbsent is atomic per key: same-language first-callers coordinate on the load,
		// different languages don't block each other
		private static final Map<String, Wordlist> CACHE = new ConcurrentHashMap<>();

		// index -> word, used to turn entropy into a mnemonic
		private final List<String> words;
		// word -> index, used to validate a mnemonic; built once per language so assertValid never rebuilds it per call
		private final Map<String, Integer> indexByWord;

		private Wordlist(final List<String> words) {
			this.words = words;

			final Map<String, Integer> index = new HashMap<>((int) (words.size() / 0.75f) + 1);
			for (int i = 0; i < words.size(); ++i)
				index.put(words.get(i), i);
			this.indexByWord = Collections.unmodifiableMap(index);
		}

		static Wordlist forLanguage(final String language) {
			return CACHE.computeIfAbsent(language.toLowerCase(Locale.ROOT), Wordlist::load);
		}

		private static Wordlist load(final String key) {
			final String resource = "/org/symbol/sdk/impl/bip39-" + key + ".txt";
			try (final InputStream in = Bip32.class.getResourceAsStream(resource)) {
				if (null == in)
					throw new IllegalArgumentException("BIP-39 wordlist not found for language: " + key);

				final List<String> words = new ArrayList<>(2048);
				try (final BufferedReader reader = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8))) {
					String line;
					while (null != (line = reader.readLine())) {
						// the spec stores wordlists NFKD-encoded; normalize defensively so matching
						// against the NFKD-normalized mnemonic is guaranteed
						final String trimmed = Normalizer.normalize(line.trim(), Normalizer.Form.NFKD);
						if (!trimmed.isEmpty())
							words.add(trimmed);
					}
				}
				if (2048 != words.size())
					throw new IllegalStateException("BIP-39 wordlist has " + words.size() + " entries, expected 2048");

				return new Wordlist(Collections.unmodifiableList(words));
			} catch (IOException ex) {
				throw new IllegalStateException(ex);
			}
		}
	}
}
