package org.symbol.sdk;

import java.nio.charset.StandardCharsets;
import java.text.Normalizer;
import java.util.Arrays;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

import network.lightsail.Language;
import network.lightsail.Mnemonic;

/**
 * BIP-32 HD key derivation (HMAC-SHA512) over BIP-39 mnemonics. Hand-rolled ed25519 SLIP-0010 with a Symbol/NEM root seed key (incl.
 * {@code "ed25519-keccak seed"}, which no library implements); the BIP-39 layer is delegated to {@code mnemonic4j}.
 */
public final class Bip32 {
	private static final int PRIVATE_KEY_SIZE = 32;
	private static final int CHAIN_CODE_SIZE = 32;

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
		final String normalizedMnemonic = Normalizer.normalize(mnemonic, Normalizer.Form.NFKD);
		final String[] words = normalizedMnemonic.split("\\s+");

		// Reject non-canonical whitespace BEFORE validation: split("\\s+") collapses runs of spaces, so a
		// double- or trailing-spaced phrase would validate yet derive a different seed than its canonical
		// form. mnemonic4j does not enforce this, so the guard stays here.
		if (words[0].isEmpty() || !normalizedMnemonic.equals(String.join(" ", words)))
			throw new IllegalArgumentException("mnemonic whitespace is not canonical (single spaces, no leading/trailing)");

		// Validate word count / checksum / membership against the language wordlist (throws IllegalArgumentException).
		mnemonicFor(mnemonicLanguage).toEntropy(Arrays.asList(words));

		// The passphrase is not normalized here on purpose: mnemonic4j's toSeed NFKD-normalizes its
		// "mnemonic" + passphrase salt internally (per BIP-39), so passing the raw passphrase derives the
		// spec-correct seed (pinned by passphraseIsNfkdNormalized).
		return fromSeed(Mnemonic.toSeed(normalizedMnemonic, null == password ? "" : password));
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
		// enforce the documented byte lengths up front: seedLength * 8 is a 32-bit int multiply, so an out-of-range value could
		// overflow to a valid strength (e.g. 536870928 * 8 wraps to 128) and silently generate a weaker-than-requested mnemonic
		if (16 != seedLength && 20 != seedLength && 24 != seedLength && 28 != seedLength && 32 != seedLength)
			throw new IllegalArgumentException(String.format("seedLength must be one of {16, 20, 24, 28, 32}, got %d", seedLength));

		// mnemonic4j.generate takes the entropy strength in bits and rejects anything outside {128, 160, 192, 224, 256}.
		final String mnemonic = mnemonicFor(mnemonicLanguage).generate(seedLength * 8);

		// mnemonic4j joins Japanese phrases with the ideographic space (U+3000); fold it to a regular space so the
		// generated phrase is space-delimited across all languages (the seed is unaffected — NFKD collapses it anyway).
		return mnemonic.replace('\u3000', ' ');
	}

	// A mnemonic4j Mnemonic is immutable but reads and parses a 2048-word wordlist file on construction; cache one per language
	private static final Map<Language, Mnemonic> MNEMONIC_BY_LANGUAGE = new ConcurrentHashMap<>();

	private static Mnemonic mnemonicFor(final String mnemonicLanguage) {
		return MNEMONIC_BY_LANGUAGE.computeIfAbsent(toLanguage(mnemonicLanguage), language -> new Mnemonic(language, null));
	}

	// Maps an SDK mnemonic-language name to the mnemonic4j Language enum (Symbol's "chinese" is BIP-39 simplified Chinese).
	private static Language toLanguage(final String mnemonicLanguage) {
		return switch (mnemonicLanguage.toLowerCase(Locale.ROOT)) {
			case "english" -> Language.ENGLISH;
			case "chinese", "chinese_simplified" -> Language.CHINESE_SIMPLIFIED;
			case "japanese" -> Language.JAPANESE;
			case "french" -> Language.FRENCH;
			case "spanish" -> Language.SPANISH;
			case "italian" -> Language.ITALIAN;
			case "korean" -> Language.KOREAN;
			default -> throw new IllegalArgumentException("unsupported mnemonic language: " + mnemonicLanguage);
		};
	}

	/** Representation of a BIP-32 node. */
	public static final class Bip32Node {
		/** Private key associated with this node. */
		public final CryptoTypes.PrivateKey privateKey;
		/**
		 * Chain code associated with this node. This is the internal buffer (used as the HMAC key for child derivation); callers must not
		 * mutate it.
		 */
		public final byte[] chainCode;

		Bip32Node(final byte[] hmacKey, final byte[] data) {
			// HMAC-SHA512 yields 64 bytes: the low 32 are the private key, the high 32 are the chain code
			final byte[] hmacResult = hmacSha512(hmacKey, data);
			this.privateKey = new CryptoTypes.PrivateKey(Arrays.copyOfRange(hmacResult, 0, PRIVATE_KEY_SIZE));
			this.chainCode = Arrays.copyOfRange(hmacResult, PRIVATE_KEY_SIZE, PRIVATE_KEY_SIZE + CHAIN_CODE_SIZE);
		}

		/**
		 * Derives a direct child node with the given identifier.
		 *
		 * @param identifier Child identifier.
		 * @return Child BIP-32 node.
		 */
		public Bip32Node deriveOne(final int identifier) {
			// SLIP-0010 hardened-derivation data: 0x00 | parent private key (32) | hardened identifier (4, big-endian). Hardening
			// forces the high bit, so the identifier region is (identifier | 0x80000000).
			final byte[] childData = new byte[1 + PRIVATE_KEY_SIZE + 4];
			System.arraycopy(privateKey.bytes(), 0, childData, 1, PRIVATE_KEY_SIZE);

			final int hardenedIdentifier = identifier | 0x80000000;
			for (int i = 0; i < 4; ++i)
				childData[childData.length - 1 - i] = (byte) (hardenedIdentifier >>> (8 * i));

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
}
