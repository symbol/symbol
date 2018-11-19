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

const { expect } = require('chai');
const test = require('../binaryTestUtils');
const EntityType = require('../../src/model/EntityType');
const ModelSchemaBuilder = require('../../src/model/ModelSchemaBuilder');
const ModelType = require('../../src/model/ModelType');
const namespace = require('../../src/plugins/namespace');
const BinarySerializer = require('../../src/serializer/BinarySerializer');

const constants = {
	sizes: {
		aliasAddress: 34,
		namespaceName: 6,
		registerNamespace: 18,
		aliasMosaic: 17,
		mosaicDefinition: 15,
		mosaicProperty: 9,
		mosaicSupplyChange: 17
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
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 12);
			expect(modelSchema).to.contain.all.keys(
				'aliasAddress',
				'namespaceDescriptor',
				'namespaceDescriptor.namespace',
				'namespaceNameTuple',
				'registerNamespace',
				'aliasMosaic',
				'mosaicDefinition',
				'mosaicDefinition.mosaicProperty',
				'mosaicDescriptor',
				'mosaicDescriptor.mosaic',
				'mosaicNameTuple',
				'mosaicSupplyChange'
			);

			// - alias address
			expect(Object.keys(modelSchema.aliasAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.aliasAddress).to.contain.all.keys(['namespaceId', 'address']);

			expect(Object.keys(modelSchema.namespaceDescriptor).length).to.equal(2);
			expect(modelSchema.namespaceDescriptor).to.contain.all.keys(['meta', 'namespace']);

			expect(Object.keys(modelSchema['namespaceDescriptor.namespace']).length).to.equal(8);
			expect(modelSchema['namespaceDescriptor.namespace']).to.contain.all.keys([
				'level0', 'level1', 'level2', 'parentId', 'owner', 'ownerAddress', 'startHeight', 'endHeight'
			]);

			expect(Object.keys(modelSchema.namespaceNameTuple).length).to.equal(3);
			expect(modelSchema.namespaceNameTuple).to.contain.all.keys(['namespaceId', 'name', 'parentId']);

			// - register namespace
			expect(Object.keys(modelSchema.registerNamespace).length).to.equal(Object.keys(modelSchema.transaction).length + 4);
			expect(modelSchema.registerNamespace).to.contain.all.keys(['namespaceId', 'parentId', 'duration', 'name']);

			// - alias mosaic
			expect(Object.keys(modelSchema.aliasMosaic).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.aliasMosaic).to.contain.all.keys(['namespaceId', 'mosaicId']);

			// - mosaic definition
			expect(Object.keys(modelSchema.mosaicDefinition).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.mosaicDefinition).to.contain.all.keys(['mosaicId', 'properties']);

			// - mosaic property
			expect(modelSchema['mosaicDefinition.mosaicProperty']).to.deep.equal({
				value: ModelType.uint64
			});

			// - mosaic descriptor
			expect(Object.keys(modelSchema.mosaicDescriptor).length).to.equal(2);
			expect(modelSchema.mosaicDescriptor).to.contain.all.keys(['meta', 'mosaic']);

			expect(Object.keys(modelSchema['mosaicDescriptor.mosaic']).length).to.equal(6);
			expect(modelSchema['mosaicDescriptor.mosaic']).to.contain.all.keys([
				'namespaceId', 'mosaicId', 'supply', 'height', 'owner', 'properties'
			]);

			// - name tuples
			expect(Object.keys(modelSchema.mosaicNameTuple).length).to.equal(3);
			expect(modelSchema.mosaicNameTuple).to.contain.all.keys(['mosaicId', 'name', 'parentId']);

			// - mosaic supply change
			expect(Object.keys(modelSchema.mosaicSupplyChange).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.mosaicSupplyChange).to.contain.all.keys(['mosaicId', 'delta']);
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
			expect(Object.keys(codecs).length).to.equal(5);
			expect(codecs).to.contain.all.keys([
				EntityType.registerNamespace.toString(),
				EntityType.aliasAddress.toString(),
				EntityType.mosaicDefinition.toString(),
				EntityType.mosaicSupplyChange.toString(),
				EntityType.aliasMosaic.toString()
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

		describe('supports mosaic definition', () => {
			const generateTransaction = () => ({
				buffer: Buffer.concat([
					Buffer.of(0x06, 0xFF, 0xCA, 0xB8), // mosaic nonce
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // mosaic id
					Buffer.of(0x00, 0x11, 0xFF) // properties header
				]),

				object: {
					nonce: 3100311302,
					mosaicId: [0x066C26F2, 0x92B28340],
					properties: [
						{ key: 0x00, value: [0x11, 0] },
						{ key: 0x01, value: [0xFF, 0] }
					]
				}
			});

			const addProperties = generator => () => {
				const data = generator();
				data.buffer = Buffer.concat([
					data.buffer,
					Buffer.of(0x02), // property key 1
					Buffer.of(0x17, 0x1A, 0x02, 0xFB, 0xD4, 0x9C, 0x73, 0x75), // property value 1
					Buffer.of(0xFF), // property key 2
					Buffer.of(0xB4, 0x64, 0x72, 0x42, 0xF1, 0xFF, 0x11, 0x00), // property value 2
					Buffer.of(0x03), // property key 3
					Buffer.of(0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0x87, 0xF8) // property value 3
				]);
				data.buffer.writeUInt8(3, constants.sizes.mosaicDefinition - 3);

				data.object.properties = data.object.properties.concat([
					{ key: 0x02, value: [0xFB021A17, 0x75739CD4] },
					{ key: 0xFF, value: [0x427264B4, 0x0011FFF1] },
					{ key: 0x03, value: [0x8F9AD09F, 0xF887353D] }
				]);
				return data;
			};

			const addAll = (generator, extraSize) => {
				const size = constants.sizes.mosaicDefinition + extraSize;
				test.binary.test.addAll(getCodec(EntityType.mosaicDefinition), size, generator);
			};

			describe('with default properties', () => {
				addAll(generateTransaction, 0);
			});

			describe('with custom properties', () => {
				addAll(addProperties(generateTransaction), 3 * constants.sizes.mosaicProperty);
			});

			describe('with invalid default properties', () => {
				const runFailureTest = (properties, errorMessage) => {
					// Arrange:
					const codec = getCodec(EntityType.mosaicDefinition);
					const serializer = new BinarySerializer(constants.sizes.mosaicDefinition);
					const transaction = generateTransaction().object;
					transaction.properties = properties;

					// Act:
					expect(() => codec.serialize(transaction, serializer)).to.throw(errorMessage);
				};

				it('fails if too few properties are present', () => {
					// Assert:
					runFailureTest([{ key: 0x00, value: [0x11, 0] }], 'all required properties must be specified in bag');
				});

				it('fails if default properties are out of order', () => {
					// Assert:
					runFailureTest(
						[{ key: 0x00, value: [0x11, 0] }, { key: 0x02, value: [0x22, 0] }, { key: 0x01, value: [0x33, 0] }],
						'unexpected property 2 at position 1 in bag'
					);
				});

				it('fails if default property is larger than byte', () => {
					// Assert:
					runFailureTest([{ key: 0x00, value: [0x11, 0] }, { key: 0x01, value: [0x100, 0] }], 'property 1 value is too large');
				});

				it('fails if default property is non-compactable', () => {
					// Assert:
					const properties = [{ key: 0x00, value: [0x100, 0x10000000] }, { key: 0x01, value: [0x22, 0] }];
					runFailureTest(properties, 'property 0 value is too large');
				});
			});
		});

		describe('supports mosaic supply change', () => {
			const generateTransaction = () => ({
				buffer: Buffer.concat([
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // mosaic id
					Buffer.of(0x01), // direction
					Buffer.of(0xCA, 0xD0, 0x8E, 0x6E, 0xFF, 0x21, 0x2F, 0x49) // delta
				]),

				object: {
					direction: 0x01,
					mosaicId: [0x066C26F2, 0x92B28340],
					delta: [0x6E8ED0CA, 0x492F21FF]
				}
			});

			const size = constants.sizes.mosaicSupplyChange;
			test.binary.test.addAll(getCodec(EntityType.mosaicSupplyChange), size, generateTransaction);
		});
	});
});
