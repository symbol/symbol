package org.symbol.sdk;

/**
 * Interface implemented by every serializable model class produced by the catbuffer generator.
 */
public interface Serializer {
	/**
	 * Returns the size, in bytes, of the serialized form of this object.
	 *
	 * @return Size in bytes.
	 */
	int size();

	/**
	 * Serializes this object to a fresh byte array.
	 *
	 * @return Serialized bytes.
	 */
	byte[] serialize();
}
