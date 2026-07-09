package org.symbol.sdk.utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Buffer view that supports moving / shrinking the visible window over a backing array without copying. Thin wrapper over a little-endian
 * {@link ByteBuffer}; the visible window is {@code [position, limit)}.
 */
public final class BufferView {
	private ByteBuffer buffer;

	/**
	 * Creates buffer view around a buffer.
	 *
	 * @param buffer Initial buffer view.
	 */
	public BufferView(final byte[] buffer) {
		this(ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN));
	}

	private BufferView(final ByteBuffer buffer) {
		this.buffer = buffer;
	}

	private static ByteBuffer rebasedSlice(final ByteBuffer source) {
		return source.slice().order(ByteOrder.LITTLE_ENDIAN);
	}

	/**
	 * Copies the first {@code size} bytes of the visible window into a fresh array, bounded to the window (a {@code size} larger than the
	 * remaining bytes throws). The current position is not advanced (the caller shiftRights by the field's size).
	 *
	 * @param size Number of bytes to copy.
	 * @return Copied bytes.
	 */
	public byte[] peekBytes(final int size) {
		validateSizeWithinWindow(size);
		final byte[] copy = new byte[size];
		buffer.get(buffer.position(), copy);
		return copy;
	}

	/**
	 * Returns the length of the visible window.
	 *
	 * @return Length.
	 */
	public int length() {
		return buffer.remaining();
	}

	/** Rejects a size outside {@code [0, remaining]}. */
	private void validateSizeWithinWindow(final int size) {
		if (0 > size)
			throw new IndexOutOfBoundsException("size cannot be negative: " + size);

		if (size > buffer.remaining())
			throw new IndexOutOfBoundsException(String.format("size %d exceeds the %d remaining bytes", size, buffer.remaining()));
	}

	/**
	 * Moves view right.
	 *
	 * @param size Amount of bytes to shift.
	 */
	public void shiftRight(final int size) {
		validateSizeWithinWindow(size);
		buffer.position(buffer.position() + size);
	}

	/**
	 * Returns a zero-copy view over the first {@code size} bytes of the visible window (sharing the backing array; this view is not
	 * advanced). The window is rebased so reads start at its first byte; copy it out with {@link #peekBytes(int)}.
	 *
	 * @param size Length in bytes.
	 * @return View limited to specified size.
	 */
	public BufferView window(final int size) {
		validateSizeWithinWindow(size);
		return new BufferView(rebasedSlice(buffer).limit(size));
	}

	/**
	 * Returns an independent, zero-copy view over this view's current window (rebased so reads start at its first byte). Reads on the
	 * returned cursor do not advance this view — used by deserializers to walk a private cursor while the caller advances the original by
	 * the object's size().
	 *
	 * @return Independent view over the same window.
	 */
	public BufferView snapshot() {
		return window(length());
	}

	/**
	 * Shrinks view to specified size.
	 *
	 * @param size New length in bytes.
	 */
	public void shrink(final int size) {
		validateSizeWithinWindow(size);
		buffer = rebasedSlice(buffer).limit(size);
	}

	/**
	 * Reads a little-endian integer of {@code size} bytes at the current position without advancing, sign- or zero-extended into a
	 * {@code long} (an 8-byte unsigned {@code u64 >= 2^63} reads back negative — see {@link Long#toUnsignedString(long)}).
	 *
	 * @param size Byte width (1, 2, 4 or 8).
	 * @param isSigned {@code true} if the value should be signed (ignored for {@code size == 8}, which keeps the raw 64-bit pattern).
	 * @return Decoded value.
	 */
	public long peekInt(final int size, final boolean isSigned) {
		validateSizeWithinWindow(size);
		return Converter.readInt(buffer, size, isSigned);
	}
}
