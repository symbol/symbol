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
const test = require('../binaryTestUtils');
const { expect } = require('chai');

const accountPropertiesPlugin = require('../../src/plugins/accountProperties');
const { PropertyType } = require('../../src/plugins/accountProperties');

describe('account properties plugin', () => {
	describe('property types enumeration', () => {
		it('contains valid values', () => {
			const propertyTypeBlockOffset = 128;

			// Assert:
			expect(PropertyType.addressAllow).to.equal(1);
			expect(PropertyType.addressBlock).to.equal(1 + propertyTypeBlockOffset);
			expect(PropertyType.mosaicAllow).to.equal(2);
			expect(PropertyType.mosaicBlock).to.equal(2 + propertyTypeBlockOffset);
			expect(PropertyType.entityTypeAllow).to.equal(4);
			expect(PropertyType.entityTypeBlock).to.equal(4 + propertyTypeBlockOffset);
		});
	});

	describe('register schema', () => {
		it('adds account properties system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;
			const modificationTypeSchemas = [
				'accountProperties.addressModificationType',
				'accountProperties.mosaicModificationType',
				'accountProperties.entityTypeModificationType'
			];
			const accountPropertySchemas = [
				'accountProperties.addressAccountProperty',
				'accountProperties.mosaicAccountProperty',
				'accountProperties.entityTypeAccountProperty'
			];

			// Act:
			accountPropertiesPlugin.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 11);
			expect(modelSchema).to.contain.all.keys([
				'accountPropertiesAddress',
				'accountPropertiesMosaic',
				'accountPropertiesEntityType',
				'accountProperties',
				'accountProperties.accountProperties'
			].concat(modificationTypeSchemas).concat(accountPropertySchemas));

			// - accountPropertiesAddress
			expect(Object.keys(modelSchema.accountPropertiesAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 1);
			expect(modelSchema.accountPropertiesAddress).to.contain.all.keys(['modifications']);

			// - accountPropertiesMosaic
			expect(Object.keys(modelSchema.accountPropertiesMosaic).length).to.equal(Object.keys(modelSchema.transaction).length + 1);
			expect(modelSchema.accountPropertiesMosaic).to.contain.all.keys(['modifications']);

			// - accountPropertiesEntityType
			expect(Object.keys(modelSchema.accountPropertiesEntityType).length).to.equal(Object.keys(modelSchema.transaction).length + 1);
			expect(modelSchema.accountPropertiesEntityType).to.contain.all.keys(['modifications']);

			// - accountProperties
			expect(Object.keys(modelSchema.accountProperties).length).to.equal(1);
			expect(modelSchema.accountProperties).to.contain.all.keys(['accountProperties']);

			// - accountProperties modification types
			modificationTypeSchemas.forEach(schema => {
				expect(Object.keys(modelSchema[schema]).length).to.equal(1);
				expect(modelSchema[schema]).to.contain.all.keys(['value']);
			});

			// - accountProperties.accountProperties
			expect(Object.keys(modelSchema['accountProperties.accountProperties']).length).to.equal(2);
			expect(modelSchema['accountProperties.accountProperties']).to.contain.all.keys(['address', 'properties']);

			// - accountProperties properties
			accountPropertySchemas.forEach(schema => {
				expect(Object.keys(modelSchema[schema]).length).to.equal(1);
				expect(modelSchema[schema]).to.contain.all.keys(['values']);
			});
		});

		describe('aggregation schemas', () => {
			it('use the correct conditional schema depending on property type', () => {
				// Arrange:
				const builder = new ModelSchemaBuilder();
				accountPropertiesPlugin.registerSchema(builder);
				const modelSchema = builder.build();
				const accountPropertiesSchema = modelSchema['accountProperties.accountProperties'].properties.schemaName;

				// Assert:
				expect(accountPropertiesSchema({ propertyType: PropertyType.addressAllow }))
					.to.equal('accountProperties.addressAccountProperty');
				expect(accountPropertiesSchema({ propertyType: PropertyType.addressBlock }))
					.to.equal('accountProperties.addressAccountProperty');
				expect(accountPropertiesSchema({ propertyType: PropertyType.mosaicAllow }))
					.to.equal('accountProperties.mosaicAccountProperty');
				expect(accountPropertiesSchema({ propertyType: PropertyType.mosaicBlock }))
					.to.equal('accountProperties.mosaicAccountProperty');
				expect(accountPropertiesSchema({ propertyType: PropertyType.entityTypeAllow }))
					.to.equal('accountProperties.entityTypeAccountProperty');
				expect(accountPropertiesSchema({ propertyType: PropertyType.entityTypeBlock }))
					.to.equal('accountProperties.entityTypeAccountProperty');
			});
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			accountPropertiesPlugin.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds account properties codecs (Address, Mosaic, EntityType)', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codecs were registered
			expect(Object.keys(codecs).length).to.equal(3);
			expect(codecs).to.contain.all.keys([
				EntityType.accountPropertiesAddress.toString(),
				EntityType.accountPropertiesMosaic.toString(),
				EntityType.accountPropertiesEntityType.toString()
			]);
		});

		const addressCodec = getCodecs()[EntityType.accountPropertiesAddress];
		const mosaicCodec = getCodecs()[EntityType.accountPropertiesMosaic];
		const entityTypeCodec = getCodecs()[EntityType.accountPropertiesEntityType];
		const addNoModificationTests = (propertyType, codec) => {
			const bufferSize = 1 + 1; // property type (in bytes) + modifications count (in bytes)
			test.binary.test.addAll(codec, bufferSize, () => ({
				buffer: Buffer.concat([
					Buffer.of(propertyType), // property type
					Buffer.of(0x00) // modifications count
				]),
				object: {
					propertyType,
					modifications: []
				}
			}));
		};
		const addModificationTests = (propertyType, codec, modBuffers, modValues) => {
			// bufferSize = property type (in bytes) + mods count (in bytes) + (each mod size in bytes) * num mods
			const bufferSize = 1 + 1 + ((1 + modBuffers[0][1].byteLength) * modBuffers.length);

			const concatModificationBuffers = buffers => Buffer.concat(buffers.map(componentBuffers => Buffer.concat(componentBuffers)));

			const splitModificationValues = values => {
				const mods = [];
				for (let i = 0; i < values.length; ++i)
					mods.push({ modificationType: values[i][0], value: values[i][1] });
				return mods;
			};

			test.binary.test.addAll(codec, bufferSize, () => ({
				buffer: Buffer.concat([
					Buffer.of(propertyType), // property type
					Buffer.of(modBuffers.length), // modifications count
					concatModificationBuffers(modBuffers) // modification types and values buffers
				]),
				object: {
					propertyType,
					modifications: splitModificationValues(modValues)
				}
			}));
		};

		describe('supports account properties address with no modifications', () => {
			addNoModificationTests(PropertyType.addressAllow, addressCodec);
		});

		describe('supports account properties mosaic with no modifications', () => {
			addNoModificationTests(PropertyType.mosaicAllow, mosaicCodec);
		});

		describe('supports account properties entityType with no modifications', () => {
			addNoModificationTests(PropertyType.entityTypeAllow, entityTypeCodec);
		});

		describe('supports account properties address with one modification', () => {
			const testAddress = test.random.bytes(test.constants.sizes.addressDecoded);
			addModificationTests(
				PropertyType.addressAllow,
				addressCodec,
				[[Buffer.of(0x00), Buffer.from(testAddress)]],
				[[0x00, testAddress]]
			);
		});

		describe('supports account properties mosaic with one modification', () => {
			addModificationTests(
				PropertyType.mosaicAllow,
				mosaicCodec,
				[[Buffer.of(0x00), Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92)]],
				[[0x00, [0x066C26F2, 0x92B28340]]]
			);
		});

		describe('supports account properties entityType with one modification', () => {
			addModificationTests(
				PropertyType.entityTypeAllow,
				entityTypeCodec,
				[[Buffer.of(0x02), Buffer.of(0xF2, 0x83)]],
				[[0x02, 0x83F2]]
			);
		});

		describe('supports account properties address with several modifications', () => {
			const testAddress1 = test.random.bytes(test.constants.sizes.addressDecoded);
			const testAddress2 = test.random.bytes(test.constants.sizes.addressDecoded);
			const testAddress3 = test.random.bytes(test.constants.sizes.addressDecoded);
			addModificationTests(
				PropertyType.addressAllow,
				addressCodec,
				[
					[Buffer.of(0xD0), Buffer.from(testAddress1)],
					[Buffer.of(0x64), Buffer.from(testAddress2)],
					[Buffer.of(0x32), Buffer.from(testAddress3)]
				],
				[
					[0xD0, testAddress1],
					[0x64, testAddress2],
					[0x32, testAddress3]
				]
			);
		});

		describe('supports account properties mosaic with several modifications', () => {
			addModificationTests(
				PropertyType.mosaicAllow,
				mosaicCodec,
				[
					[Buffer.of(0xAB), Buffer.of(0xF2, 0x10, 0x20, 0x0A, 0x40, 0x83, 0xB2, 0x92)],
					[Buffer.of(0xAE), Buffer.of(0xF3, 0x11, 0x12, 0x0B, 0x40, 0x83, 0xB2, 0xE3)],
					[Buffer.of(0x22), Buffer.of(0xF4, 0x12, 0x32, 0x0C, 0x40, 0x83, 0xB2, 0xC0)]
				],
				[
					[0xAB, [0x0A2010F2, 0x92B28340]],
					[0xAE, [0x0B1211F3, 0xE3B28340]],
					[0x22, [0x0C3212F4, 0xC0B28340]]
				]
			);
		});

		describe('supports account properties entityType with several modifications', () => {
			addModificationTests(
				PropertyType.entityTypeAllow,
				entityTypeCodec,
				[
					[Buffer.of(0x5A), Buffer.of(0xF2, 0x36)],
					[Buffer.of(0x7C), Buffer.of(0xB1, 0x83)],
					[Buffer.of(0x10), Buffer.of(0x13, 0x54)]
				],
				[
					[0x5A, 0x36F2],
					[0x7C, 0x83B1],
					[0x10, 0x5413]
				]
			);
		});
	});
});
