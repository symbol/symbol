package org.symbol.sdk.utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Buffer view.
 */
public class BufferView {
	private ByteBuffer bufferViewImpl;

	private BufferView(final byte[] buffer) {
		this.bufferViewImpl = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN);
	}

	private BufferView(final ByteBuffer buffer) {
		this.bufferViewImpl = buffer.slice();
	}

	private void validSize(final int size) {
		if (size < 0) {
			throw new IllegalArgumentException("size cannot be negative");
		}

		if (size > bufferViewImpl.remaining()) {
			throw new IllegalArgumentException("size cannot be greater than the remaining bytes");
		}
	}

	/**
	 * Moves view right.
	 *
	 * @param size Amount of bytes to shift.
	 */
	public void shiftRight(final int size) {
		validSize(size);
		this.bufferViewImpl.position(bufferViewImpl.position() + size);
	}

	/**
	 * Returns new limited view.
	 *
	 * @param size Length in bytes.
	 * @return View limited to specified size.
	 */
	public BufferView window(final int size) {
		validSize(size);
		return wrap(this.bufferViewImpl.slice().limit(size));
	}

	/**
	 * Shrinks the current view size.
	 *
	 * @param size Length in bytes.
	 */
	public void shrink(final int size) {
		validSize(size);
		this.bufferViewImpl = this.bufferViewImpl.slice().limit(size);
	}

	/**
	 * Gets the underlying buffer.
	 *
	 * @return the byte buffer.
	 */
	public ByteBuffer getBuffer() {
		return this.bufferViewImpl;
	}

	/**
	 * Get the current view content
	 *
	 * @return the byte array of the view
	 */
	public byte[] getBufferContent() {
		final int position = this.bufferViewImpl.position();
		final byte[] bytes = new byte[this.bufferViewImpl.remaining()];
		this.bufferViewImpl.get(bytes);
		this.bufferViewImpl.position(position);
		return bytes;
	}

	/**
	 * create a buffer view
	 *
	 * @param buffer buffer to create the view from
	 * @return buffer view
	 */
	public static BufferView wrap(final byte[] buffer) {
		return new BufferView(buffer);
	}

	/**
	 * create a buffer view
	 *
	 * @param view buffer to create the view from
	 * @return buffer view
	 */
	public static BufferView wrap(final ByteBuffer view) {
		return new BufferView(view);
	}
}
