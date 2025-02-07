package org.symbol.sdk.utils;

/**
 * This interface allows writing to an underlining storage.
 */
public interface Writer {

	/**
	 * Writes array into buffer.
	 *
	 * @param buffer Data to write.
	 */
	void write(final byte[] buffer);

	/**
	 * Get the underlining storage.
	 */
	byte[] getStorage();
}
