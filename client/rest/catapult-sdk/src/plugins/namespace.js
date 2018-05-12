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

/** @module plugins/namespace */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const uint64 = require('../utils/uint64');

const isNamespaceTypeRoot = namespaceType => 0 === namespaceType;

const parseString = (parser, size) => parser.buffer(size).toString('ascii');

const writeString = (serializer, str) => { serializer.writeBuffer(Buffer.from(str, 'ascii')); };

/**
 * Creates a namespace plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const namespacePlugin = {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.registerNamespace, {
			namespaceId: ModelType.uint64,
			parentId: ModelType.uint64,
			duration: ModelType.uint64,
			name: ModelType.string
		});

		builder.addTransactionSupport(EntityType.mosaicDefinition, {
			mosaicId: ModelType.uint64,
			parentId: ModelType.uint64,
			name: ModelType.string,
			properties: { type: ModelType.array, schemaName: 'mosaicDefinition.mosaicProperty' }
		});
		builder.addSchema('mosaicDefinition.mosaicProperty', {
			value: ModelType.uint64
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
			namespaceId: ModelType.uint64,
			mosaicId: ModelType.uint64,
			supply: ModelType.uint64,

			height: ModelType.uint64,
			owner: ModelType.binary,
			properties: { type: ModelType.array, schemaName: ModelType.uint64 }
		});

		builder.addSchema('namespaceDescriptor', {
			meta: { type: ModelType.object, schemaName: 'transactionMetadata' },
			namespace: { type: ModelType.object, schemaName: 'namespaceDescriptor.namespace' }
		});
		builder.addSchema('namespaceDescriptor.namespace', {
			level0: ModelType.uint64,
			level1: ModelType.uint64,
			level2: ModelType.uint64,

			parentId: ModelType.uint64,
			owner: ModelType.binary,
			ownerAddress: ModelType.binary,

			startHeight: ModelType.uint64,
			endHeight: ModelType.uint64
		});

		builder.addSchema('mosaicNameTuple', {
			mosaicId: ModelType.uint64,
			name: ModelType.string,
			parentId: ModelType.uint64
		});
		builder.addSchema('namespaceNameTuple', {
			namespaceId: ModelType.uint64,
			name: ModelType.string,
			parentId: ModelType.uint64
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.registerNamespace, {
			deserialize: parser => {
				const transaction = {};
				transaction.namespaceType = parser.uint8();
				transaction[isNamespaceTypeRoot(transaction.namespaceType) ? 'duration' : 'parentId'] = parser.uint64();
				transaction.namespaceId = parser.uint64();

				const namespaceNameSize = parser.uint8();
				transaction.name = parseString(parser, namespaceNameSize);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint8(transaction.namespaceType);
				serializer.writeUint64(isNamespaceTypeRoot(transaction.namespaceType) ? transaction.duration : transaction.parentId);
				serializer.writeUint64(transaction.namespaceId);

				serializer.writeUint8(transaction.name.length);
				writeString(serializer, transaction.name);
			}
		});

		const numRequiredProperties = 2; // flags and divisibility
		codecBuilder.addTransactionSupport(EntityType.mosaicDefinition, {
			deserialize: parser => {
				const transaction = {};
				transaction.parentId = parser.uint64();
				transaction.mosaicId = parser.uint64();

				const mosaicNameSize = parser.uint8();
				const propertiesCount = parser.uint8();

				transaction.properties = [];
				for (let i = 0; i < numRequiredProperties; ++i)
					transaction.properties.push({ key: i, value: uint64.fromUint(parser.uint8()) });

				transaction.name = parseString(parser, mosaicNameSize);

				if (0 < propertiesCount) {
					for (let i = 0; i < propertiesCount; ++i) {
						const key = parser.uint8();
						const value = parser.uint64();
						transaction.properties.push({ key, value });
					}
				}

				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.parentId);
				serializer.writeUint64(transaction.mosaicId);

				serializer.writeUint8(transaction.name.length);

				const propertiesCount = transaction.properties.length - numRequiredProperties;
				if (0 > propertiesCount)
					throw Error('all required properties must be specified in bag');

				serializer.writeUint8(propertiesCount);

				// notice that required property values are uint8 size
				for (let i = 0; i < numRequiredProperties; ++i) {
					const property = transaction.properties[i];
					if (i !== property.key)
						throw Error(`unexpected property ${property.key} at position ${i} in bag`);

					const value = uint64.compact(property.value);
					if ('number' !== typeof value || 0xFF < value)
						throw Error(`property ${property.key} value is too large`);

					serializer.writeUint8(value);
				}

				writeString(serializer, transaction.name);
				for (let i = 0; i < propertiesCount; ++i) {
					const property = transaction.properties[numRequiredProperties + i];
					serializer.writeUint8(property.key);
					serializer.writeUint64(property.value);
				}
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

module.exports = namespacePlugin;
