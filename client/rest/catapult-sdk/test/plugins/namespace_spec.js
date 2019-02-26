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

const EntityType = require('../../src/model/EntityType');
const ModelSchemaBuilder = require('../../src/model/ModelSchemaBuilder');
const ModelType = require('../../src/model/ModelType');
const namespace = require('../../src/plugins/namespace');
const schemaFormatter = require('../../src/utils/schemaFormatter');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

const constants = {
	sizes: {
		aliasAddress: 34,
		aliasMosaic: 17,
		namespaceName: 6,
		registerNamespace: 18
	}
};

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
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 9);
			expect(modelSchema).to.contain.all.keys(
				'aliasAddress',
				'aliasMosaic',
				'namespaceDescriptor',
				'namespaceDescriptor.namespace',
				'namespaceDescriptor.alias.mosaic',
				'namespaceDescriptor.alias.address',
				'namespaceDescriptor.alias.empty',
				'namespaceNameTuple',
				'registerNamespace'
			);

			// - alias address
			expect(Object.keys(modelSchema.aliasAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.aliasAddress).to.contain.all.keys(['namespaceId', 'address']);

			// - alias mosaic
			expect(Object.keys(modelSchema.aliasMosaic).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.aliasMosaic).to.contain.all.keys(['namespaceId', 'mosaicId']);

			// - namespaceDescriptor
			expect(Object.keys(modelSchema.namespaceDescriptor).length).to.equal(2);
			expect(modelSchema.namespaceDescriptor).to.contain.all.keys(['meta', 'namespace']);

			// - namespaceDescriptor.namespace
			expect(Object.keys(modelSchema['namespaceDescriptor.namespace']).length).to.equal(9);
			expect(modelSchema['namespaceDescriptor.namespace']).to.contain.all.keys([
				'level0', 'level1', 'level2', 'alias', 'parentId', 'owner', 'ownerAddress', 'startHeight', 'endHeight'
			]);

			// - namespaceDescriptor.alias.mosaic
			expect(Object.keys(modelSchema['namespaceDescriptor.alias.mosaic']).length).to.equal(1);
			expect(modelSchema['namespaceDescriptor.alias.mosaic']).to.contain.all.keys([
				'mosaicId'
			]);

			// - namespaceDescriptor.alias.address
			expect(Object.keys(modelSchema['namespaceDescriptor.alias.address']).length).to.equal(1);
			expect(modelSchema['namespaceDescriptor.alias.address']).to.contain.all.keys([
				'address'
			]);

			// - namespaceDescriptor.alias.empty
			expect(Object.keys(modelSchema['namespaceDescriptor.alias.empty']).length).to.equal(0);

			// - namespaceNameTuple
			expect(Object.keys(modelSchema.namespaceNameTuple).length).to.equal(3);
			expect(modelSchema.namespaceNameTuple).to.contain.all.keys(['namespaceId', 'name', 'parentId']);

			// - register namespace
			expect(Object.keys(modelSchema.registerNamespace).length).to.equal(Object.keys(modelSchema.transaction).length + 4);
			expect(modelSchema.registerNamespace).to.contain.all.keys(['namespaceId', 'parentId', 'duration', 'name']);
		});
	});

	describe('conditional schema', () => {
		describe('uses the correct conditional schema depending on alias type', () => {
			const formatAlias = alias => {
				// Arrange:
				const formattingRules = {
					[ModelType.none]: () => 'none',
					[ModelType.binary]: () => 'binary',
					[ModelType.uint64]: () => 'uint64',
					[ModelType.objectId]: () => 'objectId',
					[ModelType.string]: () => 'string'
				};
				const namespaceDescriptorNamespace = {
					type: null,
					depth: null,
					level0: null,
					level1: null,
					level2: null,
					alias,
					parentId: null,
					owner: null,
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
				expect(Object.keys(formattedEntity).length).to.equal(11);
				expect(formattedEntity).to.contain.all.keys([
					'type', 'depth', 'level0', 'level1', 'level2', 'alias', 'parentId', 'owner', 'ownerAddress', 'startHeight', 'endHeight'
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
					type: 'none',
					mosaicId: 'uint64'
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
					type: 'none',
					address: 'binary'
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
					type: 'none'
				});
			});
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			namespace.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds namespace codecs', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codecs were registered
			expect(Object.keys(codecs).length).to.equal(3);
			expect(codecs).to.contain.all.keys([
				EntityType.aliasAddress.toString(),
				EntityType.aliasMosaic.toString(),
				EntityType.registerNamespace.toString()
			]);
		});

		const getCodec = entityType => getCodecs()[entityType];

		describe('supports alias address', () => {
			const address = test.random.bytes(test.constants.sizes.addressDecoded);

			test.binary.test.addAll(getCodec(EntityType.aliasAddress), constants.sizes.aliasAddress, () => ({
				buffer: Buffer.concat([
					Buffer.of(0xCA), // alias action
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // namespace id
					Buffer.from(address) // address
				]),

				object: {
					aliasAction: 0xCA,
					namespaceId: [0x066C26F2, 0x92B28340],
					address
				}
			}));
		});

		describe('supports alias mosaic', () => {
			test.binary.test.addAll(getCodec(EntityType.aliasMosaic), constants.sizes.aliasMosaic, () => ({
				buffer: Buffer.concat([
					Buffer.of(0xCA), // alias action
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // namespace id
					Buffer.of(0xCA, 0xD0, 0x8E, 0x6E, 0xFF, 0x21, 0x2F, 0x49) // mosaic id
				]),

				object: {
					aliasAction: 0xCA,
					namespaceId: [0x066C26F2, 0x92B28340],
					mosaicId: [0x6E8ED0CA, 0x492F21FF]
				}
			}));
		});

		describe('supports register namespace', () => {
			const generateTransaction = namespaceType => ({
				buffer: Buffer.concat([
					Buffer.of(namespaceType), // namespace type
					Buffer.of(0xCA, 0xD0, 0x8E, 0x6E, 0xFF, 0x21, 0x2F, 0x49), // duration or parent id
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // namespace id
					Buffer.of(0x06), // namespace name size
					Buffer.of(0x6A, 0x61, 0x62, 0x6F, 0x33, 0x38) // namespace name
				]),

				object: {
					namespaceType,
					[namespaceType ? 'parentId' : 'duration']: [0x6E8ED0CA, 0x492F21FF],
					namespaceId: [0x066C26F2, 0x92B28340],
					name: 'jabo38'
				}
			});

			const addAll = namespaceType => {
				const size = constants.sizes.registerNamespace + constants.sizes.namespaceName;
				test.binary.test.addAll(getCodec(EntityType.registerNamespace), size, () => generateTransaction(namespaceType));
			};

			describe('with root type', () => {
				addAll(0x00);
			});

			describe('with child type', () => {
				addAll(0x01);
			});
		});
	});
});
