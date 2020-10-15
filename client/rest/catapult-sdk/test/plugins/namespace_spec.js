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

const EntityType = require('../../src/model/EntityType');
const ModelSchemaBuilder = require('../../src/model/ModelSchemaBuilder');
const ModelType = require('../../src/model/ModelType');
const namespace = require('../../src/plugins/namespace');
const schemaFormatter = require('../../src/utils/schemaFormatter');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

const constants = {
	sizes: {
		aliasAddress: 33,
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
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 15);
			expect(modelSchema).to.contain.all.keys(
				'aliasAddress',
				'aliasMosaic',
				'namespaces',
				'namespaceDescriptor',
				'namespaceDescriptor.meta',
				'namespaceDescriptor.namespace',
				'namespaceDescriptor.alias.mosaic',
				'namespaceDescriptor.alias.address',
				'namespaceDescriptor.alias.empty',
				'namespaceNameTuple',
				'registerNamespace',
				'mosaicNames',
				'mosaicNamesTuple',
				'accountNames',
				'accountNamesTuple'
			);

			// - alias address
			expect(Object.keys(modelSchema.aliasAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.aliasAddress).to.contain.all.keys(['namespaceId', 'address', 'aliasAction']);

			// - alias mosaic
			expect(Object.keys(modelSchema.aliasMosaic).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.aliasMosaic).to.contain.all.keys(['namespaceId', 'mosaicId', 'aliasAction']);

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
			expect(Object.keys(modelSchema['namespaceDescriptor.namespace']).length).to.equal(10);
			expect(modelSchema['namespaceDescriptor.namespace']).to.contain.all.keys([
				'registrationType', 'depth', 'level0', 'level1', 'level2', 'alias', 'parentId', 'ownerAddress', 'startHeight', 'endHeight'
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

			// - register namespace
			expect(Object.keys(modelSchema.registerNamespace).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(modelSchema.registerNamespace).to.contain.all.keys(['id', 'registrationType', 'parentId', 'duration', 'name']);

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
					[ModelType.int]: () => 'int'
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
					type: 'uint8'
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
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // namespace id
					Buffer.from(address), // address
					Buffer.of(0xCA) // alias action
				]),

				object: {
					namespaceId: [0x066C26F2, 0x92B28340],
					address,
					aliasAction: 0xCA
				}
			}));
		});

		describe('supports alias mosaic', () => {
			test.binary.test.addAll(getCodec(EntityType.aliasMosaic), constants.sizes.aliasMosaic, () => ({
				buffer: Buffer.concat([
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // namespace id
					Buffer.of(0xCA, 0xD0, 0x8E, 0x6E, 0xFF, 0x21, 0x2F, 0x49), // mosaic id
					Buffer.of(0xCA) // alias action
				]),

				object: {
					namespaceId: [0x066C26F2, 0x92B28340],
					mosaicId: [0x6E8ED0CA, 0x492F21FF],
					aliasAction: 0xCA
				}
			}));
		});

		describe('supports register namespace', () => {
			const generateTransaction = registrationType => ({
				buffer: Buffer.concat([
					Buffer.of(0xCA, 0xD0, 0x8E, 0x6E, 0xFF, 0x21, 0x2F, 0x49), // duration or parent id
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // namespace id
					Buffer.of(registrationType), // namespace type
					Buffer.of(0x06), // namespace name size
					Buffer.of(0x6A, 0x61, 0x62, 0x6F, 0x33, 0x38) // namespace name
				]),

				object: {
					[registrationType ? 'parentId' : 'duration']: [0x6E8ED0CA, 0x492F21FF],
					id: [0x066C26F2, 0x92B28340],
					registrationType,
					name: 'jabo38'
				}
			});

			const addAll = registrationType => {
				const size = constants.sizes.registerNamespace + constants.sizes.namespaceName;
				test.binary.test.addAll(getCodec(EntityType.registerNamespace), size, () => generateTransaction(registrationType));
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
