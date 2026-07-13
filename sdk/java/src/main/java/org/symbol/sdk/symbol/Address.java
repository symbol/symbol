package org.symbol.sdk.symbol;

import java.util.Arrays;

import org.symbol.sdk.ByteArray;
import org.symbol.sdk.symbol.models.*;
import org.symbol.sdk.utils.Base32;
import org.symbol.sdk.utils.Converter;

/**
 * Represents a Symbol address; {@link #toString()} yields the 39-character base32 encoding.
 */
public final class Address extends ByteArray {
	/** Byte size of raw address. */
	public static final int SIZE = 24;

	/** Length of an encoded address string. */
	public static final int ENCODED_SIZE = 39;

	/**
	 * Creates a default-initialized (all-zero bytes) Symbol address.
	 */
	public Address() {
		this(new byte[SIZE]);
	}

	/**
	 * Creates a Symbol address from raw bytes.
	 *
	 * @param bytes Raw address bytes.
	 */
	public Address(final byte[] bytes) {
		super(bytes, SIZE);
	}

	/**
	 * Creates a Symbol address from a base32 string.
	 *
	 * @param addressString Encoded address string.
	 */
	public Address(final String addressString) {
		this(decode(addressString));
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
		if (rawValue instanceof Address typed)
			return new Address(typed);

		if (rawValue instanceof String string)
			return new Address(string);

		return new Address(org.symbol.sdk.ByteArray.toBytes(rawValue));
	}

	/**
	 * Deserializes a Symbol address from a payload; consumes exactly {@link #SIZE} bytes.
	 *
	 * @param payload Buffer to read from.
	 * @return Deserialized address.
	 */
	public static Address deserialize(final byte[] payload) {
		return deserialize(payload, 0);
	}

	/**
	 * Deserializes a Symbol address from a payload at the given offset; consumes exactly {@link #SIZE} bytes.
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

	/**
	 * Determines if this address is an alias.
	 *
	 * @return {@code true} if this address is an alias (low bit of the first byte is set).
	 */
	public boolean isAlias() {
		return 0 != (bytes()[0] & 0x01);
	}

	/**
	 * Attempts to convert this address into a namespace id.
	 *
	 * @return Namespace id if this address is an alias, {@code null} otherwise.
	 */
	public NamespaceId toNamespaceId() {
		if (!isAlias())
			return null;

		// the namespace id is the u64 in bytes[1..9]
		return new NamespaceId(Converter.bytesToInt(bytes(), 1, 8, false));
	}

	@Override
	public String toString() {
		// pad to a multiple of 8 chars (40) for base32, then drop the trailing byte
		final byte[] padded = new byte[SIZE + 1];
		System.arraycopy(bytes(), 0, padded, 0, SIZE);
		final String encoded = Base32.encode(padded);
		return encoded.substring(0, encoded.length() - 1);
	}

	/**
	 * Creates an address from a decoded address hex string (typically from REST).
	 *
	 * @param hexString Decoded address hex string.
	 * @return Equivalent address.
	 */
	public static Address fromDecodedAddressHexString(final String hexString) {
		return new Address(Converter.hexToUint8(hexString));
	}

	/**
	 * Creates an alias address that references a namespace id.
	 *
	 * @param namespaceId Namespace id.
	 * @param networkIdentifier Network identifier byte.
	 * @return Address referencing the namespace id.
	 */
	public static Address fromNamespaceId(final NamespaceId namespaceId, final byte networkIdentifier) {
		// first byte flags the alias (network identifier + 1), followed by the namespace id and the remaining bytes are left zero.
		final byte[] addressBytes = new byte[SIZE];
		addressBytes[0] = (byte) (networkIdentifier + 1);
		System.arraycopy(Converter.intToBytes(namespaceId.value(), 8), 0, addressBytes, 1, 8);
		return new Address(addressBytes);
	}

	private static byte[] decode(final String addressString) {
		final byte[] decoded = Base32.decode(addressString + "A");
		return Arrays.copyOf(decoded, decoded.length - 1);
	}
}
