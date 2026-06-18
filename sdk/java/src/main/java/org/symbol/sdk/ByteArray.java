package org.symbol.sdk;

import java.util.Arrays;

import org.symbol.sdk.utils.Converter;

/**
 * Fixed-size byte-backed value (key, hash, signature, address, …) extended by every byte-array POD. The base class owns the value semantics
 * (equals/hashCode/toString/serialize) that an interface cannot provide for {@link Object} methods.
 */
public abstract class ByteArray implements Serializer {
	private final byte[] bytes;

	/**
	 * Creates a fixed-size byte value, validating its length.
	 *
	 * @param bytes Raw bytes (defensively copied).
	 * @param size Required length.
	 */
	protected ByteArray(final byte[] bytes, final int size) {
		requireSize(bytes, size);
		this.bytes = bytes.clone();
	}

	/**
	 * Returns the raw bytes backing this value. The returned array is the internal buffer — callers must not mutate it.
	 *
	 * @return Raw bytes.
	 */
	public final byte[] bytes() {
		return bytes;
	}

	@Override
	public final int size() {
		return bytes.length;
	}

	@Override
	public final byte[] serialize() {
		return bytes.clone();
	}

	/**
	 * Returns the external string representation — hex by default.
	 *
	 * @return String representation of this value.
	 */
	@Override
	public String toString() {
		return Converter.uint8ToHex(bytes);
	}

	/**
	 * JSON-safe representation; delegates to {@link #toString()} so each subclass's external format flows through.
	 *
	 * @return String representation of this value.
	 */
	public final String toJson() {
		return toString();
	}

	@Override
	public final boolean equals(final Object other) {
		if (this == other)
			return true;

		// match by concrete type as well as bytes; mirrors the reference SDK's per-type tag so that distinct
		// same-size types (e.g. PublicKey vs Hash256, both 32 bytes) never compare equal
		if (null == other || getClass() != other.getClass())
			return false;

		return Arrays.equals(bytes, ((ByteArray) other).bytes);
	}

	@Override
	public final int hashCode() {
		return Arrays.hashCode(bytes);
	}

	// validates that bytes has the expected length
	private static void requireSize(final byte[] bytes, final int expectedSize) {
		if (expectedSize != bytes.length)
			throw new IllegalArgumentException(String.format("bytes was size %d but must be %d", bytes.length, expectedSize));
	}

	/**
	 * Convert a descriptor value ({@code byte[]} or any {@link ByteArray}) to raw bytes.
	 *
	 * @param value Raw descriptor value.
	 * @return Converted bytes.
	 */
	public static byte[] toBytes(final Object value) {
		if (value instanceof byte[] bytes)
			return bytes;

		if (value instanceof ByteArray byteArray)
			return byteArray.bytes();

		throw new InvalidDescriptorException("cannot convert " + (null == value ? "null" : value.getClass().getName()) + " to bytes");
	}
}
