package org.symbol.sdk.nem;

import java.util.Arrays;

import org.symbol.sdk.ByteArray;
import org.symbol.sdk.utils.Base32;

/**
 * Represents a NEM address; {@link #toString()} returns the base32-encoded form.
 */
public final class Address extends ByteArray {
	/** Byte size of raw address. */
	public static final int SIZE = 25;

	/** Length of encoded address string. */
	public static final int ENCODED_SIZE = 40;

	/**
	 * Creates a default-initialized (all-zero bytes) NEM address.
	 */
	public Address() {
		this(new byte[SIZE]);
	}

	/**
	 * Creates a NEM address from raw bytes.
	 *
	 * @param bytes Raw address bytes.
	 */
	public Address(final byte[] bytes) {
		super(bytes, SIZE);
	}

	/**
	 * Creates a NEM address from a base32 string.
	 *
	 * @param addressString Encoded address string.
	 */
	public Address(final String addressString) {
		this(Base32.decode(addressString));
	}

	/**
	 * Copy constructor.
	 *
	 * @param other Other address.
	 */
	public Address(final Address other) {
		this(other.bytes());
	}

	/**
	 * Parses a raw value (Address, base32 string, byte array, or SDK ByteArray) into a Address.
	 *
	 * @param rawValue Raw value.
	 * @return Parsed value.
	 */
	public static Address parse(final Object rawValue) {
		// copy like the symbol Address.parse: bytes() exposes the internal buffer, so returning the caller's instance would alias it
		if (rawValue instanceof Address typed)
			return new Address(typed);

		if (rawValue instanceof String string)
			return new Address(string);

		return new Address(org.symbol.sdk.ByteArray.toBytes(rawValue));
	}

	/**
	 * Deserializes a NEM address from a payload, consuming exactly {@link #SIZE} bytes.
	 *
	 * @param payload Buffer to read from; only the first {@link #SIZE} bytes are consumed.
	 * @return Deserialized address.
	 */
	public static Address deserialize(final byte[] payload) {
		return deserialize(payload, 0);
	}

	/**
	 * Deserializes a NEM address from a payload, consuming exactly {@link #SIZE} bytes starting at {@code offset}.
	 *
	 * @param payload Buffer to read from.
	 * @param offset Offset into the payload at which to start reading.
	 * @return Deserialized address.
	 * @throws IndexOutOfBoundsException if fewer than {@link #SIZE} bytes remain at {@code offset}.
	 */
	public static Address deserialize(final byte[] payload, final int offset) {
		// copyOfRange would zero-pad a truncated payload into a valid-looking address, so bound-check explicitly
		if (payload.length - offset < SIZE)
			throw new IndexOutOfBoundsException("payload is too small to contain an address");

		return new Address(Arrays.copyOfRange(payload, offset, offset + SIZE));
	}

	@Override
	public String toString() {
		return Base32.encode(bytes());
	}
}
