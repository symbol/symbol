package org.symbol.sdk;

import java.security.SecureRandom;

import org.symbol.sdk.utils.Converter;

/**
 * Crypto types — fixed-size byte-array values ({@link Hash256}, {@link PrivateKey}, {@link PublicKey}, {@link SharedKey256},
 * {@link Signature}) for hashes, keys and signatures.
 */
public final class CryptoTypes {
	/**
	 * Single shared {@link SecureRandom} used by every {@code random()} factory in this file; {@link SecureRandom} is thread-safe and
	 * per-call allocation is costly in batch keypair generation.
	 */
	private static final SecureRandom SECURE_RANDOM = new SecureRandom();

	private CryptoTypes() {
	}

	/** Represents a 256-bit hash. */
	public static final class Hash256 extends ByteArray {
		/** Byte size of raw hash. */
		public static final int SIZE = 32;

		/**
		 * Creates a hash from raw bytes.
		 *
		 * @param bytes Input byte array.
		 */
		public Hash256(final byte[] bytes) {
			super(bytes, SIZE);
		}

		/**
		 * Creates a hash from a hex string.
		 *
		 * @param hex Input string.
		 */
		public Hash256(final String hex) {
			this(Converter.hexToUint8(hex));
		}

		/**
		 * Parses a descriptor value (Hash256, hex string, byte array, or SDK ByteArray) into a Hash256.
		 *
		 * @param descriptorValue Raw descriptor value.
		 * @return Parsed value.
		 */
		public static Hash256 parse(final Object descriptorValue) {
			if (descriptorValue instanceof Hash256 typed)
				return typed;

			if (descriptorValue instanceof String string)
				return new Hash256(string);

			return new Hash256(ByteArray.toBytes(descriptorValue));
		}

		/**
		 * Creates a zeroed hash.
		 *
		 * @return Zeroed hash.
		 */
		public static Hash256 zero() {
			return new Hash256(new byte[SIZE]);
		}
	}

	/** Represents a private key. */
	public static final class PrivateKey extends ByteArray {
		/** Byte size of raw private key. */
		public static final int SIZE = 32;

		/**
		 * Creates a private key from raw bytes.
		 *
		 * @param bytes Input byte array.
		 */
		public PrivateKey(final byte[] bytes) {
			super(bytes, SIZE);
		}

		/**
		 * Creates a private key from a hex string.
		 *
		 * @param hex Input string.
		 */
		public PrivateKey(final String hex) {
			this(Converter.hexToUint8(hex));
		}

		/**
		 * Creates a random private key.
		 *
		 * @return Random private key.
		 */
		public static PrivateKey random() {
			final byte[] bytes = new byte[SIZE];
			SECURE_RANDOM.nextBytes(bytes);
			return new PrivateKey(bytes);
		}
	}

	/** Represents a public key. */
	public static final class PublicKey extends ByteArray {
		/** Byte size of raw public key. */
		public static final int SIZE = 32;

		/**
		 * Creates a public key from raw bytes.
		 *
		 * @param bytes Input byte array.
		 */
		public PublicKey(final byte[] bytes) {
			super(bytes, SIZE);
		}

		/**
		 * Creates a public key from a hex string.
		 *
		 * @param hex Input string.
		 */
		public PublicKey(final String hex) {
			this(Converter.hexToUint8(hex));
		}

		/**
		 * Copy constructor.
		 *
		 * @param other Other public key.
		 */
		public PublicKey(final PublicKey other) {
			this(other.bytes());
		}

		/**
		 * Parses a descriptor value (PublicKey, hex string, byte array, or SDK ByteArray) into a PublicKey.
		 *
		 * @param descriptorValue Raw descriptor value.
		 * @return Parsed value.
		 */
		public static PublicKey parse(final Object descriptorValue) {
			if (descriptorValue instanceof PublicKey typed)
				return typed;

			if (descriptorValue instanceof String string)
				return new PublicKey(string);

			return new PublicKey(ByteArray.toBytes(descriptorValue));
		}
	}

	/** Represents a 256-bit symmetric encryption key. */
	public static final class SharedKey256 extends ByteArray {
		/** Byte size of raw shared key. */
		public static final int SIZE = 32;

		/**
		 * Creates a shared key from raw bytes.
		 *
		 * @param bytes Input byte array.
		 */
		public SharedKey256(final byte[] bytes) {
			super(bytes, SIZE);
		}

		/**
		 * Creates a shared key from a hex string.
		 *
		 * @param hex Input string.
		 */
		public SharedKey256(final String hex) {
			this(Converter.hexToUint8(hex));
		}
	}

	/** Represents a signature. */
	public static final class Signature extends ByteArray {
		/** Byte size of raw signature. */
		public static final int SIZE = 64;

		/**
		 * Creates a signature from raw bytes.
		 *
		 * @param bytes Input byte array.
		 */
		public Signature(final byte[] bytes) {
			super(bytes, SIZE);
		}

		/**
		 * Creates a signature from a hex string.
		 *
		 * @param hex Input string.
		 */
		public Signature(final String hex) {
			this(Converter.hexToUint8(hex));
		}

		/**
		 * Creates a zeroed signature.
		 *
		 * @return Zeroed signature.
		 */
		public static Signature zero() {
			return new Signature(new byte[SIZE]);
		}
	}
}
