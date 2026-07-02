package org.symbol.sdk.utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Position-tracking writer over a little-endian {@link ByteBuffer}; the underlying byte array is exposed via {@link #storage()} for callers
 * that need to hand off the raw buffer.
 */
public final class Writer {
	private final byte[] storage;

	private final ByteBuffer buffer;

	/**
	 * Creates a writer with specified size.
	 *
	 * @param size Allocated buffer size.
	 */
	public Writer(final int size) {
		this.storage = new byte[size];
		this.buffer = ByteBuffer.wrap(storage).order(ByteOrder.LITTLE_ENDIAN);
	}

	/**
	 * Returns the underlying storage. This is the live backing array (the same one the {@link ByteBuffer} writes into), not a defensive
	 * copy, so callers must not mutate it while the writer is in use — it is exposed to hand off the raw buffer without copying.
	 *
	 * @return Backing byte array (aliased, not copied).
	 */
	public byte[] storage() {
		return storage;
	}

	/**
	 * Returns the current write offset (number of bytes written so far); derived from the buffer position, so it can never drift.
	 *
	 * @return Current write offset.
	 */
	public int offset() {
		return buffer.position();
	}

	private void validateRemainingSize(final int size) {
		if (size > buffer.remaining())
			throw new IndexOutOfBoundsException(
					String.format("write of %d bytes would exceed buffer (remaining %d)", size, buffer.remaining()));
	}

	/**
	 * Writes array into buffer.
	 *
	 * @param data Data to write.
	 */
	public void write(final byte[] data) {
		validateRemainingSize(data.length);
		buffer.put(data);
	}

	/**
	 * Writes a little-endian integer into the buffer.
	 *
	 * @param value Value to write.
	 * @param size Byte width (1, 2, 4 or 8).
	 */
	public void writeInt(final long value, final int size) {
		Converter.validateByteSize(size);

		// reject silent truncation
		if (8 != size) {
			final long mask = (1L << (8 * size)) - 1;
			final long low = value & mask;
			final long signExtended = 0 != (low & (1L << (8 * size - 1))) ? low | ~mask : low;
			if (value != low && value != signExtended)
				throw new IllegalArgumentException(String.format("value %d does not fit in %d byte(s)", value, size));
		}

		validateRemainingSize(size);
		switch (size) {
			case 1 -> buffer.put((byte) value);
			case 2 -> buffer.putShort((short) value);
			case 4 -> buffer.putInt((int) value);
			case 8 -> buffer.putLong(value);
			default -> throw new IllegalArgumentException(String.format("unsupported int size %d", size));
		}
	}
}
