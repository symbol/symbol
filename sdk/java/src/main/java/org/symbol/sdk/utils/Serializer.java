package org.symbol.sdk.utils;

/**
 * This interface serialize a catbuffer object.
 */
public interface Serializer {

	/**
	 * Serializes an object to bytes.
	 *
	 * @return Serialized bytes.
	 */
	byte[] serialize();

	/**
	 * Gets the size of the object.
	 *
	 * @return Size in bytes.
	 */
	int getSize();
}
