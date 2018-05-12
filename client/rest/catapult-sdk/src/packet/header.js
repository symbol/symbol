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

/** @exports packet/header */
const packetHeader = {
	/**
	 * @property {numeric} size The size (in bytes) of a packet header.
	 */
	size: 8,

	/**
	 * Creates a packet header buffer.
	 * @param {module:packet/types} type The packet type.
	 * @param {numeric} size The packet size.
	 * @returns {Buffer} The packet header buffer.
	 */
	createBuffer: (type, size) => {
		const header = Buffer.alloc(packetHeader.size);
		header.writeInt32LE(size, 0);
		header.writeInt32LE(type, 4);
		return header;
	}
};

module.exports = packetHeader;
