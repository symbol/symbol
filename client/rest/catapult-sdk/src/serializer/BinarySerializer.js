/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

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
		buffer.forEach(byte => {
			this.writeUint8(byte);
		});
	}
}

/**
 * Provides an interface for writing to a fixed size buffer.
 */
class BinarySerializer {
	/**
	 * Creates a binary serializer.
	 * @param {numeric} size Size of the underlying fixed size buffer.
	 */
	constructor(size) {
		if (!Number.isInteger(size) || 0 >= size)
			throw Error('BinarySerializer constructor needs integer size > 0');

		this.container = new BufferContainer(size);
	}

	/**
	 * Gets the size of the underlying fixed size buffer.
	 * @returns {Numeric} Size of the underlying buffer.
	 */
	bufferSize() {
		return this.container.buffer.length;
	}

	/**
	 * Gets the underlying fixed size buffer.
	 * @returns {Buffer} Underlying buffer.
	 */
	buffer() {
		return this.container.buffer;
	}

	/**
	 * Writes a uint8 to the working buffer.
	 * @param {numeric} value Value to write.
	 */
	writeUint8(value) {
		this.container.writeUint8(value);
	}

	/**
	 * Writes a uint16 to the working buffer.
	 * @param {numeric} value Value to write.
	 */
	writeUint16(value) {
		this.container.writeUint16(value);
	}

	/**
	 * Writes a uint32 to the working buffer.
	 * @param {numeric} value Value to write.
	 */
	writeUint32(value) {
		this.container.writeUint32(value);
	}

	/**
	 * Writes a uint64 to the working buffer.
	 * @param {numeric} value Value to write.
	 */
	writeUint64(value) {
		this.container.writeUint64(value);
	}

	/**
	 * Writes a buffer of bytes to the working buffer.
	 * @param {Buffer} buffer Buffer to write.
	 */
	writeBuffer(buffer) {
		this.container.writeBuffer(buffer);
	}
}

module.exports = BinarySerializer;
