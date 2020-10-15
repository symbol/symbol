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
			namespaceId: ModelType.uint64HexIdentifier,
			address: ModelType.binary,
			aliasAction: ModelType.uint8
		});

		builder.addTransactionSupport(EntityType.aliasMosaic, {
			namespaceId: ModelType.uint64HexIdentifier,
			mosaicId: ModelType.uint64HexIdentifier,
			aliasAction: ModelType.uint8
		});

		builder.addTransactionSupport(EntityType.registerNamespace, {
			id: ModelType.uint64HexIdentifier,
			registrationType: ModelType.uint8,
			parentId: ModelType.uint64HexIdentifier,
			duration: ModelType.uint64,
			name: ModelType.string
		});

		builder.addSchema('namespaces', {
			namespaces: { type: ModelType.array, schemaName: 'namespaceDescriptor' }
		});

		builder.addSchema('namespaceDescriptor', {
			id: ModelType.objectId,
			meta: { type: ModelType.object, schemaName: 'namespaceDescriptor.meta' },
			namespace: { type: ModelType.object, schemaName: 'namespaceDescriptor.namespace' }
		});

		builder.addSchema('namespaceDescriptor.meta', {
			active: ModelType.boolean,
			index: ModelType.int
		});

		builder.addSchema('namespaceDescriptor.namespace', {
			registrationType: ModelType.uint8,
			depth: ModelType.uint8,
			level0: ModelType.uint64HexIdentifier,
			level1: ModelType.uint64HexIdentifier,
			level2: ModelType.uint64HexIdentifier,

			alias: { type: ModelType.object, schemaName: entity => getAliasBasicType(entity.type) },

			parentId: ModelType.uint64HexIdentifier,
			ownerAddress: ModelType.binary,

			startHeight: ModelType.uint64,
			endHeight: ModelType.uint64
		});

		builder.addSchema('namespaceDescriptor.alias.mosaic', {
			type: ModelType.uint8,
			mosaicId: ModelType.uint64HexIdentifier
		});

		builder.addSchema('namespaceDescriptor.alias.address', {
			type: ModelType.uint8,
			address: ModelType.binary
		});

		builder.addSchema('namespaceDescriptor.alias.empty', {
			type: ModelType.uint8
		});

		builder.addSchema('namespaceNameTuple', {
			id: ModelType.uint64HexIdentifier,
			name: ModelType.string,
			parentId: ModelType.uint64HexIdentifier
		});

		builder.addSchema('mosaicNames', {
			mosaicNames: { type: ModelType.array, schemaName: 'mosaicNamesTuple' }
		});

		builder.addSchema('mosaicNamesTuple', {
			mosaicId: ModelType.uint64HexIdentifier,
			names: { type: ModelType.array, schemaName: ModelType.string }
		});

		builder.addSchema('accountNames', {
			accountNames: { type: ModelType.array, schemaName: 'accountNamesTuple' }
		});

		builder.addSchema('accountNamesTuple', {
			address: ModelType.binary,
			names: { type: ModelType.array, schemaName: ModelType.string }
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.aliasAddress, {
			deserialize: parser => ({
				namespaceId: parser.uint64(),
				address: parser.buffer(constants.sizes.addressDecoded),
				aliasAction: parser.uint8()
			}),

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.namespaceId);
				serializer.writeBuffer(transaction.address);
				serializer.writeUint8(transaction.aliasAction);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.aliasMosaic, {
			deserialize: parser => ({
				namespaceId: parser.uint64(),
				mosaicId: parser.uint64(),
				aliasAction: parser.uint8()
			}),

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.namespaceId);
				serializer.writeUint64(transaction.mosaicId);
				serializer.writeUint8(transaction.aliasAction);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.registerNamespace, {
			deserialize: parser => {
				const transaction = {};
				const parentIdOrDuration = parser.uint64();
				transaction.id = parser.uint64();
				transaction.registrationType = parser.uint8();
				transaction[isNamespaceTypeRoot(transaction.registrationType) ? 'duration' : 'parentId'] = parentIdOrDuration;
				const nameSize = parser.uint8();
				transaction.name = parseString(parser, nameSize);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(isNamespaceTypeRoot(transaction.registrationType) ? transaction.duration : transaction.parentId);
				serializer.writeUint64(transaction.id);
				serializer.writeUint8(transaction.registrationType);
				serializer.writeUint8(transaction.name.length);
				writeString(serializer, transaction.name);
			}
		});
	}
};

module.exports = namespacePlugin;
