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

/** @module modelBinary/blockHeaderCodec */
const sizes = require('./sizes');

const constants = { sizes };

const blockHeaderCodec = {
	/**
	 * Parses a block header.
	 * @param {object} parser Parser.
	 * @returns {object} Parsed block header.
	 */
	deserialize: parser => {
		const blockHeader = {};
		blockHeader.height = parser.uint64();
		blockHeader.timestamp = parser.uint64();
		blockHeader.difficulty = parser.uint64();
		blockHeader.proofGamma = parser.buffer(constants.sizes.vrfProof.gamma);
		blockHeader.proofVerificationHash = parser.buffer(constants.sizes.vrfProof.verificationHash);
		blockHeader.proofScalar = parser.buffer(constants.sizes.vrfProof.scalar);
		blockHeader.previousBlockHash = parser.buffer(constants.sizes.hash256);
		blockHeader.transactionsHash = parser.buffer(constants.sizes.hash256);
		blockHeader.receiptsHash = parser.buffer(constants.sizes.hash256);
		blockHeader.stateHash = parser.buffer(constants.sizes.hash256);
		blockHeader.beneficiaryAddress = parser.buffer(constants.sizes.addressDecoded);
		blockHeader.feeMultiplier = parser.uint32();
		blockHeader.blockHeader_Reserved1 = parser.uint32();
		return blockHeader;
	},

	/**
	 * Serializes a block header.
	 * @param {object} blockHeader Block header.
	 * @param {object} serializer Serializer.
	 */
	serialize: (blockHeader, serializer) => {
		serializer.writeUint64(blockHeader.height);
		serializer.writeUint64(blockHeader.timestamp);
		serializer.writeUint64(blockHeader.difficulty);
		serializer.writeBuffer(blockHeader.proofGamma);
		serializer.writeBuffer(blockHeader.proofVerificationHash);
		serializer.writeBuffer(blockHeader.proofScalar);
		serializer.writeBuffer(blockHeader.previousBlockHash);
		serializer.writeBuffer(blockHeader.transactionsHash);
		serializer.writeBuffer(blockHeader.receiptsHash);
		serializer.writeBuffer(blockHeader.stateHash);
		serializer.writeBuffer(blockHeader.beneficiaryAddress);
		serializer.writeUint32(blockHeader.feeMultiplier);
		serializer.writeUint32(blockHeader.blockHeader_Reserved1);
	}
};

module.exports = blockHeaderCodec;
