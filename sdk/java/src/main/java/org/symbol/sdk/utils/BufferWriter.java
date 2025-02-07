package org.symbol.sdk.utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Writer use for serialization.
 */
public class BufferWriter implements Writer {
	final ByteBuffer storage;

	/**
	 * Creates a writer with specified size.
	 *
	 * @param size Allocated buffer size.
	 */
	public BufferWriter(final int size) {
		this.storage = ByteBuffer.allocate(size).order(ByteOrder.LITTLE_ENDIAN);
	}

	/**
	 * Writes array into buffer.
	 *
	 * @param buffer Data to write.
	 */
	public void write(byte[] buffer) {
		this.storage.put(buffer);
	}

	/**
	 * Get the underlining array.
	 *
	 * @return array buffer
	 */
	public byte[] getStorage() {
		return this.storage.array();
	}
}
