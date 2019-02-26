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
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

const isNamespaceTypeRoot = namespaceType => 0 === namespaceType;

const parseString = (parser, size) => parser.buffer(size).toString('ascii');

const writeString = (serializer, str) => { serializer.writeBuffer(Buffer.from(str, 'ascii')); };

const AliasType = {
	1: 'namespaceDescriptor.alias.mosaic',
	2: 'namespaceDescriptor.alias.address'
};

const getAliasBasicType = type => AliasType[type] || 'namespaceDescriptor.alias.empty';

/**
 * Creates a namespace plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const namespacePlugin = {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.aliasAddress, {
			namespaceId: ModelType.uint64,
			address: ModelType.binary
		});

		builder.addTransactionSupport(EntityType.aliasMosaic, {
			namespaceId: ModelType.uint64,
			mosaicId: ModelType.uint64
		});

		builder.addTransactionSupport(EntityType.registerNamespace, {
			namespaceId: ModelType.uint64,
			parentId: ModelType.uint64,
			duration: ModelType.uint64,
			name: ModelType.string
		});

		builder.addSchema('namespaceDescriptor', {
			meta: { type: ModelType.object, schemaName: 'transactionMetadata' },
			namespace: { type: ModelType.object, schemaName: 'namespaceDescriptor.namespace' }
		});
		builder.addSchema('namespaceDescriptor.namespace', {
			level0: ModelType.uint64,
			level1: ModelType.uint64,
			level2: ModelType.uint64,

			alias: { type: ModelType.object, schemaName: entity => getAliasBasicType(entity.type) },

			parentId: ModelType.uint64,
			owner: ModelType.binary,
			ownerAddress: ModelType.binary,

			startHeight: ModelType.uint64,
			endHeight: ModelType.uint64
		});

		builder.addSchema('namespaceDescriptor.alias.mosaic', {
			mosaicId: ModelType.uint64
		});

		builder.addSchema('namespaceDescriptor.alias.address', {
			address: ModelType.binary
		});

		builder.addSchema('namespaceDescriptor.alias.empty', {});

		builder.addSchema('namespaceNameTuple', {
			namespaceId: ModelType.uint64,
			name: ModelType.string,
			parentId: ModelType.uint64
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.aliasAddress, {
			deserialize: parser => ({
				aliasAction: parser.uint8(),
				namespaceId: parser.uint64(),
				address: parser.buffer(constants.sizes.addressDecoded)
			}),

			serialize: (transaction, serializer) => {
				serializer.writeUint8(transaction.aliasAction);
				serializer.writeUint64(transaction.namespaceId);
				serializer.writeBuffer(transaction.address);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.aliasMosaic, {
			deserialize: parser => ({
				aliasAction: parser.uint8(),
				namespaceId: parser.uint64(),
				mosaicId: parser.uint64()
			}),

			serialize: (transaction, serializer) => {
				serializer.writeUint8(transaction.aliasAction);
				serializer.writeUint64(transaction.namespaceId);
				serializer.writeUint64(transaction.mosaicId);
			}
		});

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
	}
};

module.exports = namespacePlugin;
