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
import ModelType from '../model/ModelType.js';
import { models } from 'symbol-sdk/symbol';

const AliasType = {
	1: 'namespaceDescriptor.alias.mosaic',
	2: 'namespaceDescriptor.alias.address'
};

const getAliasBasicType = type => AliasType[type] || 'namespaceDescriptor.alias.empty';

/**
 * Creates a namespace plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addTransactionSupport(models.TransactionType.ADDRESS_ALIAS, {
			namespaceId: ModelType.uint64HexIdentifier,
			address: ModelType.encodedAddress,
			aliasAction: ModelType.uint8
		});

		builder.addTransactionSupport(models.TransactionType.MOSAIC_ALIAS, {
			namespaceId: ModelType.uint64HexIdentifier,
			mosaicId: ModelType.uint64HexIdentifier,
			aliasAction: ModelType.uint8
		});

		builder.addTransactionSupport(models.TransactionType.NAMESPACE_REGISTRATION, {
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
			version: ModelType.uint16,
			registrationType: ModelType.uint8,
			depth: ModelType.uint8,
			level0: ModelType.uint64HexIdentifier,
			level1: ModelType.uint64HexIdentifier,
			level2: ModelType.uint64HexIdentifier,

			alias: { type: ModelType.object, schemaName: entity => getAliasBasicType(entity.type) },

			parentId: ModelType.uint64HexIdentifier,
			ownerAddress: ModelType.encodedAddress,

			startHeight: ModelType.uint64,
			endHeight: ModelType.uint64
		});

		builder.addSchema('namespaceDescriptor.alias.mosaic', {
			type: ModelType.uint8,
			mosaicId: ModelType.uint64HexIdentifier
		});

		builder.addSchema('namespaceDescriptor.alias.address', {
			type: ModelType.uint8,
			address: ModelType.encodedAddress
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
			address: ModelType.encodedAddress,
			names: { type: ModelType.array, schemaName: ModelType.string }
		});
	}
};
