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

/** @module plugins/lockSecret */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

/**
 * Creates a lock secret plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const lockSecretPlugin = {
	registerSchema: builder => {
		builder.addSchema('secretLockInfo', {
			id: ModelType.objectId,
			lock: { type: ModelType.object, schemaName: 'secretLockInfo.lock' }
		});
		builder.addSchema('secretLockInfo.lock', {
			ownerAddress: ModelType.binary,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64,
			endHeight: ModelType.uint64,
			status: ModelType.uint8,
			hashAlgorithm: ModelType.uint8,
			secret: ModelType.binary,
			recipientAddress: ModelType.binary,
			compositeHash: ModelType.binary
		});

		builder.addTransactionSupport(EntityType.secretLock, {
			recipientAddress: ModelType.binary,
			secret: ModelType.binary,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64,
			duration: ModelType.uint64,
			hashAlgorithm: ModelType.uint8
		});
		builder.addTransactionSupport(EntityType.secretProof, {
			secret: ModelType.binary,
			recipientAddress: ModelType.binary,
			proof: ModelType.binary,
			hashAlgorithm: ModelType.uint8
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.secretLock, {
			deserialize: parser => {
				const transaction = {};
				transaction.recipientAddress = parser.buffer(constants.sizes.addressDecoded);
				transaction.secret = parser.buffer(constants.sizes.hash256);
				transaction.mosaicId = parser.uint64();
				transaction.amount = parser.uint64();
				transaction.duration = parser.uint64();
				transaction.hashAlgorithm = parser.uint8();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeBuffer(transaction.recipientAddress);
				serializer.writeBuffer(transaction.secret);
				serializer.writeUint64(transaction.mosaicId);
				serializer.writeUint64(transaction.amount);
				serializer.writeUint64(transaction.duration);
				serializer.writeUint8(transaction.hashAlgorithm);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.secretProof, {
			deserialize: parser => {
				const transaction = {};
				transaction.recipientAddress = parser.buffer(constants.sizes.addressDecoded);
				transaction.secret = parser.buffer(constants.sizes.hash256);
				const proofSize = parser.uint16();
				transaction.hashAlgorithm = parser.uint8();
				transaction.proof = parser.buffer(proofSize);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeBuffer(transaction.recipientAddress);
				serializer.writeBuffer(transaction.secret);
				serializer.writeUint16(transaction.proof.length);
				serializer.writeUint8(transaction.hashAlgorithm);
				serializer.writeBuffer(transaction.proof);
			}
		});
	}
};

module.exports = lockSecretPlugin;
