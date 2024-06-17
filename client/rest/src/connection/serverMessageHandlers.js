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

import catapult from '../catapult-sdk/index.js';
import { Hash256 } from 'symbol-sdk';
import { models } from 'symbol-sdk/symbol';

const { uint64 } = catapult.utils;

const parserFromData = binaryData => {
	const parser = new catapult.parser.BinaryParser();
	parser.push(binaryData);
	return parser;
};

const fixupTransactionJson = transactionJson => {
	if (transactionJson.mosaics) {
		transactionJson.mosaics = transactionJson.mosaics.map(mosaic => ({
			id: mosaic.mosaicId,
			amount: mosaic.amount
		}));
	}

	if (transactionJson.mosaic) {
		Object.assign(transactionJson, transactionJson.mosaic);
		delete transactionJson.mosaic;
	}

	if (transactionJson.transactions) {
		transactionJson.transactions = transactionJson.transactions.map(subTransaction => ({
			transaction: fixupTransactionJson(subTransaction)
		}));
	}

	return transactionJson;
};

export default Object.freeze({
	block: emit => (topic, binaryBlock, hash, generationHash) => {
		// rewrite block size to block header size, which is necessary for parser to work
		const uint32Array = new Uint32Array(1);
		uint32Array[0] = binaryBlock.length;
		const uint8Size = catapult.utils.convert.uint32ToUint8(uint32Array);

		const block = models.BlockFactory.deserialize(new Uint8Array([
			...uint8Size,
			...binaryBlock.subarray(4)
		]));
		emit({ type: 'blockHeaderWithMetadata', payload: { block: block.toJson(), meta: { hash, generationHash } } });
	},

	finalizedBlock: emit => (topic, binaryBlock) => {
		const parser = parserFromData(binaryBlock);

		const finalizationEpoch = parser.uint32();
		const finalizationPoint = parser.uint32();
		const height = parser.uint64();
		const hash = parser.buffer(Hash256.SIZE);
		emit({
			type: 'finalizedBlock',
			payload: {
				finalizationEpoch, finalizationPoint, height, hash
			}
		});
	},

	transaction: emit => (topic, binaryTransaction, hash, merkleComponentHash, height) => {
		const transaction = models.TransactionFactory.deserialize(binaryTransaction);
		const meta = { hash, merkleComponentHash, height: uint64.fromBytes(height) };

		const transactionJson = fixupTransactionJson(transaction.toJson());
		emit({ type: 'transactionWithMetadata', payload: { transaction: transactionJson, meta } });
	},

	transactionHash: emit => (topic, hash) => {
		emit({ type: 'transactionWithMetadata', payload: { meta: { hash } } });
	},

	transactionStatus: emit => (topic, buffer) => {
		const parser = parserFromData(buffer);

		const hash = parser.buffer(Hash256.SIZE);
		const deadline = parser.uint64();
		const code = parser.uint32();
		emit({ type: 'transactionStatus', payload: { hash, code, deadline } });
	}
});
