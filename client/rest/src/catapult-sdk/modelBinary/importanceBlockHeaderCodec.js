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

/** @module modelBinary/importanceBlockHeaderCodec */
const sizes = require('./sizes');

const constants = { sizes };

const importanceBlockHeaderCodec = {
	/**
	 * Parses a block header.
	 * @param {object} parser Parser.
	 * @returns {object} Parsed block header.
	 */
	deserialize: parser => {
		const importanceBlockHeader = {};
		importanceBlockHeader.votingEligibleAccountsCount = parser.uint32();
		importanceBlockHeader.harvestingEligibleAccountsCount = parser.uint64();
		importanceBlockHeader.totalVotingBalance = parser.uint64();
		importanceBlockHeader.previousImportanceBlockHash = parser.buffer(constants.sizes.hash256);
		return importanceBlockHeader;
	},

	/**
	 * Serializes a block header.
	 * @param {object} importanceBlockHeader Block header.
	 * @param {object} serializer Serializer.
	 */
	serialize: (importanceBlockHeader, serializer) => {
		serializer.writeUint32(importanceBlockHeader.votingEligibleAccountsCount);
		serializer.writeUint64(importanceBlockHeader.harvestingEligibleAccountsCount);
		serializer.writeUint64(importanceBlockHeader.totalVotingBalance);
		serializer.writeBuffer(importanceBlockHeader.previousImportanceBlockHash);
	}
};

module.exports = importanceBlockHeaderCodec;
