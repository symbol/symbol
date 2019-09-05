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
			id: ModelType.uint64,
			duration: ModelType.uint64
		});

		builder.addTransactionSupport(EntityType.mosaicSupplyChange, {
			mosaicId: ModelType.uint64,
			delta: ModelType.uint64
		});

		builder.addSchema('mosaicDescriptor', {
			meta: { type: ModelType.object, schemaName: 'transactionMetadata' },
			mosaic: { type: ModelType.object, schemaName: 'mosaicDescriptor.mosaic' }
		});

		builder.addSchema('mosaicDescriptor.mosaic', {
			id: ModelType.uint64,
			supply: ModelType.uint64,

			startHeight: ModelType.uint64,
			ownerPublicKey: ModelType.binary,
			ownerAddress: ModelType.binary,
			revision: 'number',
			properties: { type: ModelType.object, schemaName: 'mosaicDefinition.mosaicProperties' }
		});

		builder.addSchema('mosaicDefinition.mosaicProperties', {
			duration: ModelType.uint64
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.mosaicDefinition, {
			deserialize: parser => {
				const transaction = {};
				transaction.nonce = parser.uint32();
				transaction.id = parser.uint64();
				transaction.flags = parser.uint8();
				transaction.divisibility = parser.uint8();
				transaction.duration = parser.uint64();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint32(transaction.nonce);
				serializer.writeUint64(transaction.id);
				serializer.writeUint8(transaction.flags);
				serializer.writeUint8(transaction.divisibility);
				serializer.writeUint64(transaction.duration);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.mosaicSupplyChange, {
			deserialize: parser => {
				const transaction = {};
				transaction.mosaicId = parser.uint64();
				transaction.direction = parser.uint8();
				transaction.delta = parser.uint64();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.mosaicId);
				serializer.writeUint8(transaction.direction);
				serializer.writeUint64(transaction.delta);
			}
		});
	}
};

module.exports = mosaicPlugin;
