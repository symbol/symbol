package org.symbol.sdk.utils;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Position-tracking writer over a little-endian {@link ByteBuffer}; the underlying byte array is exposed via {@link #storage} for callers
 * that need to hand off the raw buffer.
 */
public final class Writer {
	/**
	 * Underlying storage; aliases the {@link ByteBuffer}'s backing array, so helper writes are visible here without copying.
	 */
	public final byte[] storage;

	/**
	 * Current write offset; updated after every write so callers can read the field directly.
	 */
	public int offset;

	private final ByteBuffer buffer;

	/**
	 * Creates a writer with specified size.
	 *
	 * @param size Allocated buffer size.
	 */
	public Writer(final int size) {
		this.storage = new byte[size];
		this.buffer = ByteBuffer.wrap(storage).order(ByteOrder.LITTLE_ENDIAN);
		this.offset = 0;
	}

	/**
	 * Writes array into buffer.
	 *
	 * @param data Data to write.
	 */
	public void write(final byte[] data) {
		if (data.length > buffer.remaining())
			throw new IndexOutOfBoundsException(
					String.format("write of %d bytes would exceed buffer (remaining %d)", data.length, buffer.remaining()));

		buffer.put(data);
		offset = buffer.position();
	}

	/**
	 * Writes a little-endian integer into the buffer.
	 *
	 * @param value Value to write.
	 * @param size Byte width (1, 2 or 4).
	 */
	public void writeInt(final long value, final int size) {
		switch (size) {
			case 1 -> buffer.put((byte) value);
			case 2 -> buffer.putShort((short) value);
			case 4 -> buffer.putInt((int) value);
			default -> throw new IllegalArgumentException(String.format("unsupported int size %d", size));
		}
		offset = buffer.position();
	}

	/**
	 * Writes a little-endian 8-byte {@link BigInteger} (signed or unsigned) into the buffer.
	 *
	 * @param value Value to write.
	 * @param size Byte width (must be 8).
	 */
	public void writeBigInt(final BigInteger value, final int size) {
		if (8 != size)
			throw new IllegalArgumentException(String.format("unsupported big int size %d", size));

		// longValue() keeps the low 64 bits; with LITTLE_ENDIAN putLong this matches the byte
		// order Converter.intToBytes produces for both signed and unsigned inputs
		buffer.putLong(value.longValue());
		offset = buffer.position();
	}
}
