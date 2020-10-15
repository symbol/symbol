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

const catapult = require('catapult-sdk');

const { uint64 } = catapult.utils;

const parserFromData = binaryData => {
	const parser = new catapult.parser.BinaryParser();
	parser.push(binaryData);
	return parser;
};

const ServerMessageHandler = Object.freeze({
	block: (codec, emit) => (topic, binaryBlock, hash, generationHash) => {
		const block = codec.deserialize(parserFromData(binaryBlock));
		emit({ type: 'blockHeaderWithMetadata', payload: { block, meta: { hash, generationHash } } });
	},

	finalizedBlock: (codec, emit) => (topic, binaryBlock) => {
		const parser = parserFromData(binaryBlock);

		const finalizationEpoch = parser.uint32();
		const finalizationPoint = parser.uint32();
		const height = parser.uint64();
		const hash = parser.buffer(catapult.constants.sizes.hash256);
		emit({
			type: 'finalizedBlock',
			payload: {
				finalizationEpoch, finalizationPoint, height, hash
			}
		});
	},

	transaction: (codec, emit) => (topic, binaryTransaction, hash, merkleComponentHash, height) => {
		const transaction = codec.deserialize(parserFromData(binaryTransaction));
		const meta = { hash, merkleComponentHash, height: uint64.fromBytes(height) };
		emit({ type: 'transactionWithMetadata', payload: { transaction, meta } });
	},

	transactionHash: (codec, emit) => (topic, hash) => {
		emit({ type: 'transactionWithMetadata', payload: { meta: { hash } } });
	},

	transactionStatus: (codec, emit) => (topic, buffer) => {
		const parser = parserFromData(buffer);

		const hash = parser.buffer(catapult.constants.sizes.hash256);
		const deadline = parser.uint64();
		const code = parser.uint32();
		emit({ type: 'transactionStatus', payload: { hash, code, deadline } });
	}
});

module.exports = {
	ServerMessageHandler
};
