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

/** @module plugins/lock */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

const commonLockInfoSchema = {
	account: ModelType.binary,
	accountAddress: ModelType.binary,
	mosaicId: ModelType.uint64,
	amount: ModelType.uint64,
	height: ModelType.uint64
	/* status */
};

const commonLockTransactionSchema = {
	mosaicId: ModelType.uint64,
	amount: ModelType.uint64,
	duration: ModelType.uint64
};

const parseLockData = (parser, transaction) => {
	transaction.mosaicId = parser.uint64();
	transaction.amount = parser.uint64();
	transaction.duration = parser.uint64();
};

const serializeLockData = (transaction, serializer) => {
	serializer.writeUint64(transaction.mosaicId);
	serializer.writeUint64(transaction.amount);
	serializer.writeUint64(transaction.duration);
};

/**
 * Creates a lock plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const lockPlugin = {
	registerSchema: builder => {
		builder.addSchema('hashLockInfo', {
			lock: { type: ModelType.object, schemaName: 'hashLockInfo.lock' }
		});
		builder.addSchema('hashLockInfo.lock', Object.assign({}, commonLockInfoSchema, {
			hash: ModelType.binary
		}));

		builder.addSchema('secretLockInfo', {
			lock: { type: ModelType.object, schemaName: 'secretLockInfo.lock' }
		});
		builder.addSchema('secretLockInfo.lock', Object.assign({}, commonLockInfoSchema, {
			/* hashAlgorithm */
			secret: ModelType.binary,
			recipient: ModelType.binary
		}));

		builder.addTransactionSupport(EntityType.hashLock, Object.assign({}, commonLockTransactionSchema, {
			hash: ModelType.binary
		}));
		builder.addTransactionSupport(EntityType.secretLock, Object.assign({}, commonLockTransactionSchema, {
			secret: ModelType.binary,
			recipient: ModelType.binary
		}));
		builder.addTransactionSupport(EntityType.secretProof, {
			secret: ModelType.binary,
			proof: ModelType.binary
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.hashLock, {
			deserialize: parser => {
				const transaction = {};
				parseLockData(parser, transaction);

				transaction.hash = parser.buffer(constants.sizes.hash256);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializeLockData(transaction, serializer);

				serializer.writeBuffer(transaction.hash);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.secretLock, {
			deserialize: parser => {
				const transaction = {};
				parseLockData(parser, transaction);

				transaction.hashAlgorithm = parser.uint8();
				transaction.secret = parser.buffer(constants.sizes.hash256);
				transaction.recipient = parser.buffer(constants.sizes.addressDecoded);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializeLockData(transaction, serializer);

				serializer.writeUint8(transaction.hashAlgorithm);
				serializer.writeBuffer(transaction.secret);
				serializer.writeBuffer(transaction.recipient);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.secretProof, {
			deserialize: parser => {
				const transaction = {};
				transaction.hashAlgorithm = parser.uint8();
				transaction.secret = parser.buffer(constants.sizes.hash256);
				const proofSize = parser.uint16();
				transaction.proof = parser.buffer(proofSize);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint8(transaction.hashAlgorithm);
				serializer.writeBuffer(transaction.secret);
				const proofSize = transaction.proof.length;
				serializer.writeUint16(proofSize);
				serializer.writeBuffer(transaction.proof);
			}
		});
	}
};

module.exports = lockPlugin;
