/** @module serializer/BinarySerializer */

class BufferContainer {
	constructor(size) {
		this.buffer = Buffer.alloc(size, 0);
		this.offset = 0;
	}

	requireBufferSpace(size) {
		const bytesLeft = this.buffer.length - this.offset;
		if (size > bytesLeft)
			throw Error(`insufficient buffer space left (${size} bytes required, ${bytesLeft} bytes available)`);
	}

	writeUint8(value) {
		this.requireBufferSpace(1);
		this.buffer.writeUInt8(value, this.offset);
		++this.offset;
	}

	writeUint16(value) {
		this.requireBufferSpace(2);
		this.buffer.writeUInt16LE(value, this.offset);
		this.offset += 2;
	}

	writeUint32(value) {
		this.requireBufferSpace(4);
		this.buffer.writeUInt32LE(value, this.offset);
		this.offset += 4;
	}

	writeUint64(value) {
		this.requireBufferSpace(8);
		this.writeUint32(value[0]);
		this.writeUint32(value[1]);
	}

	writeBuffer(buffer) {
		this.requireBufferSpace(buffer.length);
		for (const byte of buffer)
			this.writeUint8(byte);
	}
}

/**
 * Provides an interface for writing to a fixed size buffer.
 */
export default class BinarySerializer {
	/**
	 * Creates a binary serializer.
	 * @param {numeric} size The size of the underlying fixed size buffer.
	 */
	constructor(size) {
		if (!Number.isInteger(size) || 0 >= size)
			throw Error('BinarySerializer constructor needs integer size > 0');

		this.container = new BufferContainer(size);
	}

	/**
	 * Gets the size of the underlying fixed size buffer.
	 * @returns {Numeric} The size of the underlying buffer.
	 */
	bufferSize() {
		return this.container.buffer.length;
	}

	/**
	 * Gets the underlying fixed size buffer.
	 * @returns {Buffer} The underlying buffer.
	 */
	buffer() {
		return this.container.buffer;
	}

	/**
	 * Writes a uint8 to the working buffer.
	 * @param {numeric} value The value to write.
	 */
	writeUint8(value) {
		this.container.writeUint8(value);
	}

	/**
	 * Writes a uint16 to the working buffer.
	 * @param {numeric} value The value to write.
	 */
	writeUint16(value) {
		this.container.writeUint16(value);
	}

	/**
	 * Writes a uint32 to the working buffer.
	 * @param {numeric} value The value to write.
	 */
	writeUint32(value) {
		this.container.writeUint32(value);
	}

	/**
	 * Writes a uint64 to the working buffer.
	 * @param {numeric} value The value to write.
	 */
	writeUint64(value) {
		this.container.writeUint64(value);
	}

	/**
	 * Writes a buffer of bytes to the working buffer.
	 * @param {Buffer} buffer The buffer to write.
	 */
	writeBuffer(buffer) {
		this.container.writeBuffer(buffer);
	}
}
