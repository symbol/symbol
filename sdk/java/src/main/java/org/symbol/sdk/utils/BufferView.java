package org.symbol.sdk.utils;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Buffer view that supports moving / shrinking the visible window over a backing array without copying. Thin wrapper over a little-endian
 * {@link ByteBuffer}; the visible window is {@code [position, limit)}.
 */
public final class BufferView {
	private final byte[] backing;
	private final ByteBuffer buffer;

	/**
	 * Creates buffer view around a buffer.
	 *
	 * @param buffer Initial buffer view.
	 */
	public BufferView(final byte[] buffer) {
		this(buffer, 0, buffer.length);
	}

	/**
	 * Creates a buffer view over a window of a backing array, without copying.
	 *
	 * @param backing Backing array.
	 * @param offset Offset of the visible window in the backing array.
	 * @param length Length of the visible window.
	 */
	public BufferView(final byte[] backing, final int offset, final int length) {
		this.backing = backing;
		this.buffer = ByteBuffer.wrap(backing, offset, length).order(ByteOrder.LITTLE_ENDIAN);
	}

	/**
	 * Returns a copy of the currently visible bytes.
	 *
	 * @return Visible bytes.
	 */
	public byte[] buffer() {
		final byte[] copy = new byte[buffer.remaining()];
		buffer.duplicate().get(copy);
		return copy;
	}

	/**
	 * Returns the underlying backing array. Callers must use {@link #offset()} and {@link #length()}.
	 *
	 * @return Backing array (not copied).
	 */
	public byte[] backing() {
		return backing;
	}

	/**
	 * Returns the offset of the visible window in the backing array.
	 *
	 * @return Offset.
	 */
	public int offset() {
		return buffer.position();
	}

	/**
	 * Returns the length of the visible window.
	 *
	 * @return Length.
	 */
	public int length() {
		return buffer.remaining();
	}

	/**
	 * Moves view right.
	 *
	 * @param size Amount of bytes to shift.
	 */
	public void shiftRight(final int size) {
		requireNonNegative(size);
		final int newPosition = buffer.position() + size;
		if (newPosition > buffer.limit())
			throw new IndexOutOfBoundsException(String.format("shift of %d would exceed buffer (remaining %d)", size, buffer.remaining()));

		buffer.position(newPosition);
	}

	/** Rejects negative sizes up front so corrupted size arithmetic fails with a clear message. */
	private static void requireNonNegative(final int size) {
		if (0 > size)
			throw new IndexOutOfBoundsException("size cannot be negative: " + size);
	}

	/**
	 * Returns a new limited view.
	 *
	 * @param size Length in bytes.
	 * @return View limited to specified size.
	 */
	public byte[] window(final int size) {
		requireNonNegative(size);
		if (size > buffer.remaining())
			throw new IndexOutOfBoundsException(String.format("invalid shrink value: %d vs %d", size, buffer.remaining()));

		final byte[] copy = new byte[size];
		buffer.duplicate().get(copy, 0, size);
		return copy;
	}

	/**
	 * Shrinks view to specified size.
	 *
	 * @param size New length in bytes.
	 */
	public void shrink(final int size) {
		requireNonNegative(size);
		if (size > buffer.remaining())
			throw new IndexOutOfBoundsException(String.format("invalid shrink value: %d vs %d", size, buffer.remaining()));

		buffer.limit(buffer.position() + size);
	}

	/**
	 * Reads a little-endian integer at the current position without advancing.
	 *
	 * @param size Byte width (1, 2 or 4).
	 * @param isSigned {@code true} if the value should be treated as signed.
	 * @return Decoded value.
	 */
	public long peekInt(final int size, final boolean isSigned) {
		final int position = buffer.position();
		long raw = switch (size) {
			case 1 -> buffer.get(position) & 0xFFL;
			case 2 -> buffer.getShort(position) & 0xFFFFL;
			case 4 -> buffer.getInt(position) & 0xFFFFFFFFL;
			default -> throw new IllegalArgumentException(String.format("unsupported int size %d", size));
		};
		if (isSigned) {
			final long signMask = 1L << (8 * size - 1);
			if (0 != (raw & signMask))
				raw -= 1L << (8 * size);
		}

		return raw;
	}

	/**
	 * Reads a little-endian 8-byte integer at the current position without advancing.
	 *
	 * @param size Byte width (must be 8).
	 * @param isSigned {@code true} if the value should be treated as signed.
	 * @return Decoded value.
	 */
	public BigInteger peekBigInt(final int size, final boolean isSigned) {
		if (8 != size)
			throw new IllegalArgumentException(String.format("unsupported int size %d", size));

		final long raw = buffer.getLong(buffer.position());
		BigInteger value = BigInteger.valueOf(raw);
		// Java's `long` is always signed; flip negative results into the unsigned u64 range when asked.
		if (!isSigned && 0 > raw)
			value = value.add(BigInteger.ONE.shiftLeft(64));

		return value;
	}
}
