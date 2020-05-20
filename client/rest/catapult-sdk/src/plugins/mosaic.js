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

/** @module plugins/mosaic */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');

/**
 * Creates a mosaic plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const mosaicPlugin = {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.mosaicDefinition, {
			id: ModelType.uint64HexIdentifier,
			duration: ModelType.uint64
		});

		builder.addTransactionSupport(EntityType.mosaicSupplyChange, {
			mosaicId: ModelType.uint64HexIdentifier,
			delta: ModelType.uint64
		});

		builder.addSchema('mosaicDescriptor', {
			id: ModelType.objectId,
			mosaic: { type: ModelType.object, schemaName: 'mosaicDescriptor.mosaic' }
		});

		builder.addSchema('mosaicDescriptor.mosaic', {
			id: ModelType.uint64HexIdentifier,
			supply: ModelType.uint64,
			startHeight: ModelType.uint64,
			ownerPublicKey: ModelType.binary,
			ownerAddress: ModelType.binary,
			duration: ModelType.uint64
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.mosaicDefinition, {
			deserialize: parser => {
				const transaction = {};
				transaction.id = parser.uint64();
				transaction.duration = parser.uint64();
				transaction.nonce = parser.uint32();
				transaction.flags = parser.uint8();
				transaction.divisibility = parser.uint8();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.id);
				serializer.writeUint64(transaction.duration);
				serializer.writeUint32(transaction.nonce);
				serializer.writeUint8(transaction.flags);
				serializer.writeUint8(transaction.divisibility);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.mosaicSupplyChange, {
			deserialize: parser => {
				const transaction = {};
				transaction.mosaicId = parser.uint64();
				transaction.delta = parser.uint64();
				transaction.action = parser.uint8();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.mosaicId);
				serializer.writeUint64(transaction.delta);
				serializer.writeUint8(transaction.action);
			}
		});
	}
};

module.exports = mosaicPlugin;
