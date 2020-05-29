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
const metadataPlugin = require('../../src/plugins/metadata');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

describe('metadata plugin', () => {
	describe('register schema', () => {
		it('adds metadata system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			metadataPlugin.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 6);
			expect(modelSchema).to.contain.all.keys([
				'accountMetadata',
				'mosaicMetadata',
				'namespaceMetadata',
				'metadata',
				'metadata.entry',
				'metadata.entry.element'
			]);

			// - accountMetadata
			expect(Object.keys(modelSchema.accountMetadata).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.accountMetadata).to.contain.all.keys(['targetAddress', 'scopedMetadataKey', 'value']);

			// - mosaicMetadata
			expect(Object.keys(modelSchema.mosaicMetadata).length).to.equal(Object.keys(modelSchema.transaction).length + 4);
			expect(modelSchema.mosaicMetadata).to.contain.all.keys(['targetAddress', 'scopedMetadataKey', 'targetMosaicId', 'value']);

			// - namespaceMetadata
			expect(Object.keys(modelSchema.namespaceMetadata).length).to.equal(Object.keys(modelSchema.transaction).length + 4);
			expect(modelSchema.namespaceMetadata).to.contain.all.keys([
				'targetAddress',
				'scopedMetadataKey',
				'targetNamespaceId',
				'value'
			]);

			// - metadata
			expect(Object.keys(modelSchema.metadata).length).to.equal(1);
			expect(modelSchema.metadata).to.contain.all.keys(['metadataEntries']);

			// - metadata.entry
			expect(Object.keys(modelSchema['metadata.entry']).length).to.equal(2);
			expect(modelSchema['metadata.entry']).to.contain.all.keys(['metadataEntry', 'id']);

			// - metadata.entry.element
			expect(Object.keys(modelSchema['metadata.entry.element']).length).to.equal(6);
			expect(modelSchema['metadata.entry.element']).to.contain.all.keys([
				'compositeHash',
				'sourceAddress',
				'targetAddress',
				'scopedMetadataKey',
				'targetId',
				'value'
			]);
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			metadataPlugin.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds metadata codecs', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codecs were registered
			expect(Object.keys(codecs).length).to.equal(3);
			expect(codecs).to.contain.all.keys([
				EntityType.accountMetadata.toString(),
				EntityType.mosaicMetadata.toString(),
				EntityType.namespaceMetadata.toString()
			]);
		});

		const getCodec = entityType => getCodecs()[entityType];

		describe('supports account metadata', () => {
			const targetPublicKey = test.random.bytes(test.constants.sizes.signerPublicKey); // 32
			const valueBuffer = Buffer.of(0x6d, 0x65, 0x74, 0x61, 0x20, 0x69, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e);

			test.binary.test.addAll(getCodec(EntityType.accountMetadata), 60, () => ({
				buffer: Buffer.concat([
					Buffer.from(targetPublicKey), // key 32b
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // scopedMetadataKey 8b
					Buffer.of(0x03, 0x00), // valueSizeDelta
					Buffer.of(0x10, 0x00), // valueSize
					valueBuffer // value 16b
				]),

				object: {
					targetPublicKey,
					scopedMetadataKey: [0x066C26F2, 0x92B28340],
					valueSizeDelta: 3,
					value: valueBuffer
				}
			}));
		});

		describe('supports mosaic metadata', () => {
			const targetPublicKey = test.random.bytes(test.constants.sizes.signerPublicKey); // 32
			const valueBuffer = Buffer.of(0x6d, 0x65, 0x74, 0x61, 0x20, 0x69, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e);

			test.binary.test.addAll(getCodec(EntityType.mosaicMetadata), 68, () => ({
				buffer: Buffer.concat([
					Buffer.from(targetPublicKey), // key 32b
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // scopedMetadataKey 8b
					Buffer.of(0x93, 0x53, 0xBB, 0x24, 0x12, 0xB1, 0xFF, 0x36), // targetMosaicId 8b
					Buffer.of(0x05, 0x00), // valueSizeDelta
					Buffer.of(0x10, 0x00), // valueSize
					valueBuffer // value 16b
				]),

				object: {
					targetPublicKey,
					scopedMetadataKey: [0x066C26F2, 0x92B28340],
					targetMosaicId: [0x24BB5393, 0x36FFB112],
					valueSizeDelta: 5,
					value: valueBuffer
				}
			}));
		});

		describe('supports namespace metadata', () => {
			const targetPublicKey = test.random.bytes(test.constants.sizes.signerPublicKey); // 32
			const valueBuffer = Buffer.of(0x6d, 0x65, 0x74, 0x61, 0x20, 0x69, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x69, 0x6f, 0x6e);

			test.binary.test.addAll(getCodec(EntityType.namespaceMetadata), 68, () => ({
				buffer: Buffer.concat([
					Buffer.from(targetPublicKey), // key 32b
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92), // scopedMetadataKey 8b
					Buffer.of(0xAA, 0x22, 0xC2, 0x32, 0x99, 0xBC, 0xDE, 0x63), // targetNamespaceId 8b
					Buffer.of(0x12, 0x00), // valueSizeDelta
					Buffer.of(0x10, 0x00), // valueSize
					valueBuffer // value 16b
				]),

				object: {
					targetPublicKey,
					scopedMetadataKey: [0x066C26F2, 0x92B28340],
					targetNamespaceId: [0x32C222AA, 0x63DEBC99],
					valueSizeDelta: 18,
					value: valueBuffer
				}
			}));
		});
	});
});
