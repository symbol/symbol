/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

/** @module parser/BinaryParser */

class BufferContainer {
	constructor() {
		this.offset = 0;
		this.buffers = [];
		this.numUnprocessedBytes = 0;
	}

	current() {
		return this.buffers[0];
	}

	push(buffer) {
		if (!Buffer.isBuffer(buffer))
			throw Error('BinaryParser only supports aggregating NodeJS Buffer objects');

		if (0 === buffer.length)
			return;

		this.buffers.push(buffer);
		this.numUnprocessedBytes += buffer.length;
	}

	consume(size) {
		this.offset += size;
		this.numUnprocessedBytes -= size;

		if (this.offset === this.current().length) {
			this.offset = 0;
			this.buffers.shift();
		}
	}

	requireUnprocessed(size) {
		if (this.numUnprocessedBytes < size)
			throw Error(`insufficient unprocessed data (${size} bytes required, ${this.numUnprocessedBytes} bytes available)`);
	}

	nextByte() {
		this.requireUnprocessed(1);

		const buffer = this.current();
		const byte = buffer[this.offset];
		this.consume(1);
		return byte;
	}

	nextInteger(size) {
		this.requireUnprocessed(size);

		let value = 0;
		for (let i = 0; i < size; ++i)
			value |= this.nextByte() << (i * 8);

		return value >>> 0;
	}

	nextBuffer(size) {
		this.requireUnprocessed(size);

		return this.nextBufferInPlace(size) || this.nextBufferOutOfPlace(size);
	}

	nextBufferInPlace(size) {
		const buffer = this.current();
		if (buffer.length - this.offset < size)
			return undefined;

		const result = buffer.slice(this.offset, this.offset + size);
		this.consume(size);
		return result;
	}

	nextBufferOutOfPlace(size) {
		// the requested buffer spans multiple buffers, so copy into a new buffer
		const result = Buffer.allocUnsafe(size);
		for (let i = 0; i < size; ++i)
			result[i] = this.nextByte();

		return result;
	}

	size() {
		return this.numUnprocessedBytes;
	}
}

/**
 * Accepts and buffers binary data and provides an interface for reading from it.
 */
class BinaryParser {
	/**
	 * Creates a binary parser.
	 */
	constructor() {
		this.buffers = new BufferContainer();
	}

	/**
	 * Accepts a binary buffer and appends it to the end of the working buffer.
	 * @param {Buffer} buffer The binary buffer.
	 */
	push(buffer) {
		this.buffers.push(buffer);
	}

	/**
	 * Reads a uint8 from the working buffer.
	 * @returns {numeric} The read uint8.
	 */
	uint8() {
		return this.buffers.nextByte();
	}

	/**
	 * Reads a uint16 from the working buffer.
	 * @returns {numeric} The read uint16.
	 */
	uint16() {
		return this.buffers.nextInteger(2);
	}

	/**
	 * Reads a uint32 from the working buffer.
	 * @returns {numeric} The read uint32.
	 */
	uint32() {
		return this.buffers.nextInteger(4);
	}

	/**
	 * Reads a uint64 from the working buffer.
	 * @returns {module:utils/uint64~uint64} The read uint64.
	 */
	uint64() {
		this.buffers.requireUnprocessed(8);
		return [this.uint32(), this.uint32()];
	}

	/**
	 * Reads a specific number of bytes from the working buffer.
	 * @param {numeric} size The number of bytes to read.
	 * @returns {Buffer} The read bytes.
	 */
	buffer(size) {
		return this.buffers.nextBuffer(size);
	}

	/**
	 * Gets the number of unprocessed bytes remaining in the working buffer.
	 * @returns {numeric} The number of unprocessed bytes.
	 */
	numUnprocessedBytes() {
		return this.buffers.size();
	}
}

module.exports = BinaryParser;
