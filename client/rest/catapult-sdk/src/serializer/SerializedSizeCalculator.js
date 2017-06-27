/** @module serializer/SerializedSizeCalculator */

/**
 * Calculates serialized size using builder pattern.
 */
export default class SerializedSizeCalculator {
	/**
	 * Creates a serialized size calculator.
	 */
	constructor() {
		this.totalSize = 0;
	}

	/**
	 * Gets the calculated size.
	 * @returns {numeric} The calculated size.
	 */
	size() {
		return this.totalSize;
	}

	/**
	 * Writes a uint8 to the working buffer.
	 */
	writeUint8() {
		++this.totalSize;
	}

	/**
	 * Writes a uint16 to the working buffer.
	 */
	writeUint16() {
		this.totalSize += 2;
	}

	/**
	 * Writes a uint32 to the working buffer.
	 */
	writeUint32() {
		this.totalSize += 4;
	}

	/**
	 * Writes a uint64 to the working buffer.
	 */
	writeUint64() {
		this.totalSize += 8;
	}

	/**
	 * Writes a buffer of bytes to the working buffer.
	 * @param {Buffer} buffer The buffer to write.
	 */
	writeBuffer(buffer) {
		this.totalSize += buffer.length;
	}
}
