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

/** @module modelBinary/blockHeaderCodec */
const sizes = require('./sizes');

const constants = { sizes };

const blockHeaderCodec = {
	/**
	 * Parses a block header.
	 * @param {object} parser The parser.
	 * @returns {object} The parsed block header.
	 */
	deserialize: parser => {
		const blockHeader = {};
		blockHeader.height = parser.uint64();
		blockHeader.timestamp = parser.uint64();
		blockHeader.difficulty = parser.uint64();
		blockHeader.previousBlockHash = parser.buffer(constants.sizes.hash256);
		blockHeader.blockTransactionsHash = parser.buffer(constants.sizes.hash256);
		return blockHeader;
	},

	/**
	 * Serializes a block header.
	 * @param {object} blockHeader The block header.
	 * @param {object} serializer The serializer.
	 */
	serialize: (blockHeader, serializer) => {
		serializer.writeUint64(blockHeader.height);
		serializer.writeUint64(blockHeader.timestamp);
		serializer.writeUint64(blockHeader.difficulty);
		serializer.writeBuffer(blockHeader.previousBlockHash);
		serializer.writeBuffer(blockHeader.blockTransactionsHash);
	}
};

module.exports = blockHeaderCodec;
