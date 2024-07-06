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

import ModelSchemaBuilder from '../../../src/catapult-sdk/model/ModelSchemaBuilder.js';
import ModelType from '../../../src/catapult-sdk/model/ModelType.js';
import namespace from '../../../src/catapult-sdk/plugins/namespace.js';
import schemaFormatter from '../../../src/catapult-sdk/utils/schemaFormatter.js';
import { expect } from 'chai';

describe('namespace plugin', () => {
	describe('register schema', () => {
		it('adds namespace system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			namespace.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 15);
			expect(modelSchema).to.contain.all.keys(
				'TransactionType.ADDRESS_ALIAS',
				'TransactionType.MOSAIC_ALIAS',
				'TransactionType.NAMESPACE_REGISTRATION',
				'namespaces',
				'namespaceDescriptor',
				'namespaceDescriptor.meta',
				'namespaceDescriptor.namespace',
				'namespaceDescriptor.alias.mosaic',
				'namespaceDescriptor.alias.address',
				'namespaceDescriptor.alias.empty',
				'namespaceNameTuple',
				'mosaicNames',
				'mosaicNamesTuple',
				'accountNames',
				'accountNamesTuple'
			);

			// - TransactionType.ADDRESS_ALIAS
			const addressAliasSchema = modelSchema['TransactionType.ADDRESS_ALIAS'];
			expect(Object.keys(addressAliasSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(addressAliasSchema).to.contain.all.keys(['namespaceId', 'address', 'aliasAction']);

			// - TransactionType.MOSAIC_ALIAS
			const mosaicAliasSchema = modelSchema['TransactionType.MOSAIC_ALIAS'];
			expect(Object.keys(mosaicAliasSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(mosaicAliasSchema).to.contain.all.keys(['namespaceId', 'mosaicId', 'aliasAction']);

			// - TransactionType.NAMESPACE_REGISTRATION
			const namespaceRegistrationSchema = modelSchema['TransactionType.NAMESPACE_REGISTRATION'];
			expect(Object.keys(namespaceRegistrationSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(namespaceRegistrationSchema).to.contain.all.keys(['id', 'registrationType', 'parentId', 'duration', 'name']);

			// - namespaces
			expect(Object.keys(modelSchema.namespaces).length).to.equal(1);
			expect(modelSchema.namespaces).to.contain.all.keys(['namespaces']);

			// - namespaceDescriptor
			expect(Object.keys(modelSchema.namespaceDescriptor).length).to.equal(3);
			expect(modelSchema.namespaceDescriptor).to.contain.all.keys(['id', 'meta', 'namespace']);

			// - namespaceDescriptor.meta
			expect(Object.keys(modelSchema['namespaceDescriptor.meta']).length).to.equal(2);
			expect(modelSchema['namespaceDescriptor.meta']).to.contain.all.keys(['active', 'index']);

			// - namespaceDescriptor.namespace
			expect(Object.keys(modelSchema['namespaceDescriptor.namespace']).length).to.equal(11);
			expect(modelSchema['namespaceDescriptor.namespace']).to.contain.all.keys([
				'version', 'registrationType', 'depth', 'level0', 'level1', 'level2',
				'alias', 'parentId', 'ownerAddress', 'startHeight', 'endHeight'
			]);

			// - namespaceDescriptor.alias.mosaic
			expect(Object.keys(modelSchema['namespaceDescriptor.alias.mosaic']).length).to.equal(2);
			expect(modelSchema['namespaceDescriptor.alias.mosaic']).to.contain.all.keys(['type', 'mosaicId']);

			// - namespaceDescriptor.alias.address
			expect(Object.keys(modelSchema['namespaceDescriptor.alias.address']).length).to.equal(2);
			expect(modelSchema['namespaceDescriptor.alias.address']).to.contain.all.keys(['type', 'address']);

			// - namespaceDescriptor.alias.empty
			expect(Object.keys(modelSchema['namespaceDescriptor.alias.empty']).length).to.equal(1);
			expect(modelSchema['namespaceDescriptor.alias.empty']).to.contain.all.keys(['type']);

			// - namespaceNameTuple
			expect(Object.keys(modelSchema.namespaceNameTuple).length).to.equal(3);
			expect(modelSchema.namespaceNameTuple).to.contain.all.keys(['id', 'name', 'parentId']);

			// - mosaic names
			expect(Object.keys(modelSchema.mosaicNames).length).to.equal(1);
			expect(modelSchema.mosaicNames).to.contain.all.keys(['mosaicNames']);

			// - mosaic names tuple
			expect(Object.keys(modelSchema.mosaicNamesTuple).length).to.equal(2);
			expect(modelSchema.mosaicNamesTuple).to.contain.all.keys(['mosaicId', 'names']);

			// - account names
			expect(Object.keys(modelSchema.accountNames).length).to.equal(1);
			expect(modelSchema.accountNames).to.contain.all.keys(['accountNames']);

			// - account names tuple
			expect(Object.keys(modelSchema.accountNamesTuple).length).to.equal(2);
			expect(modelSchema.accountNamesTuple).to.contain.all.keys(['address', 'names']);
		});
	});

	describe('conditional schema', () => {
		describe('uses the correct conditional schema depending on alias type', () => {
			const formatAlias = alias => {
				// Arrange:
				const formattingRules = {
					[ModelType.none]: () => 'none',
					[ModelType.binary]: () => 'binary',
					[ModelType.uint8]: () => 'uint8',
					[ModelType.uint16]: () => 'uint16',
					[ModelType.uint32]: () => 'uint32',
					[ModelType.uint64]: () => 'uint64',
					[ModelType.uint64HexIdentifier]: () => 'uint64HexIdentifier',
					[ModelType.objectId]: () => 'objectId',
					[ModelType.string]: () => 'string',
					[ModelType.int]: () => 'int',
					[ModelType.encodedAddress]: () => 'encodedAddress'
				};
				const namespaceDescriptorNamespace = {
					registrationType: null,
					depth: null,
					level0: null,
					level1: null,
					level2: null,
					alias,
					parentId: null,
					ownerAddress: null,
					startHeight: null,
					endHeight: null
				};
				const builder = new ModelSchemaBuilder();

				// Act:
				namespace.registerSchema(builder);
				const modelSchema = builder.build();
				const formattedEntity = schemaFormatter.format(
					namespaceDescriptorNamespace,
					modelSchema['namespaceDescriptor.namespace'],
					modelSchema,
					formattingRules
				);

				// Assert
				expect(Object.keys(formattedEntity).length).to.equal(10);
				expect(formattedEntity).to.contain.all.keys([
					'registrationType', 'depth', 'level0', 'level1', 'level2', 'alias',
					'parentId', 'ownerAddress', 'startHeight', 'endHeight'
				]);
				return formattedEntity.alias;
			};

			it('formats alias mosaic type', () => {
				// Arrange:
				const aliasMosaic = {
					type: 1,
					mosaicId: null
				};

				// Act:
				const formattedAlias = formatAlias(aliasMosaic);

				// Assert:
				expect(formattedAlias).to.contain.all.keys(['type', 'mosaicId']);
				expect(formattedAlias).deep.equal({
					type: 'uint8',
					mosaicId: 'uint64HexIdentifier'
				});
			});

			it('formats alias address type', () => {
				// Arrange:
				const aliasAddress = {
					type: 2,
					address: null
				};

				// Act:
				const formattedAlias = formatAlias(aliasAddress);

				// Assert:
				expect(formattedAlias).to.contain.all.keys(['type', 'address']);
				expect(formattedAlias).deep.equal({
					type: 'uint8',
					address: 'encodedAddress'
				});
			});

			it('formats alias empty type', () => {
				// Arrange:
				const aliasAddress = {
					type: null
				};

				// Act:
				const formattedAlias = formatAlias(aliasAddress);

				// Assert:
				expect(formattedAlias).to.contain.all.keys(['type']);
				expect(formattedAlias).deep.equal({
					type: 'uint8'
				});
			});
		});
	});
});
