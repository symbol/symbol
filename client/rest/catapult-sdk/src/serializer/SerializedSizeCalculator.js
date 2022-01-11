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

/** @module serializer/SerializedSizeCalculator */

/**
 * Calculates serialized size using builder pattern.
 */
class SerializedSizeCalculator {
	/**
	 * Creates a serialized size calculator.
	 */
	constructor() {
		this.totalSize = 0;
	}

	/**
	 * Gets the calculated size.
	 * @returns {numeric} Calculated size.
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
	 * @param {Buffer} buffer Buffer to write.
	 */
	writeBuffer(buffer) {
		this.totalSize += buffer.length;
	}
}

module.exports = SerializedSizeCalculator;
