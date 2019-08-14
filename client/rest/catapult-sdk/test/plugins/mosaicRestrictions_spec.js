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
const mosaicRestrictionsPlugin = require('../../src/plugins/mosaicRestrictions');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

describe('mosaic restrictions plugin', () => {
	describe('register schema', () => {
		it('adds mosaic restrictions system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			mosaicRestrictionsPlugin.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 2);
			expect(modelSchema).to.contain.all.keys(
				'mosaicRestrictionAddress',
				'mosaicRestrictionGlobal'
			);

			// - mosaic restriction address
			expect(Object.keys(modelSchema.mosaicRestrictionAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(modelSchema.mosaicRestrictionAddress).to.contain.all.keys([
				'mosaicId',
				'restrictionKey',
				'targetAddress',
				'previousRestrictionValue',
				'newRestrictionValue'
			]);

			// - mosaic restriction global
			expect(Object.keys(modelSchema.mosaicRestrictionGlobal).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(modelSchema.mosaicRestrictionGlobal).to.contain.all.keys([
				'mosaicId',
				'referenceMosaicId',
				'restrictionKey',
				'previousRestrictionValue',
				'newRestrictionValue'
			]);
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			mosaicRestrictionsPlugin.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});
			return codecs;
		};

		it('adds mosaic restriction codecs (Address, Global)', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codecs were registered
			expect(Object.keys(codecs).length).to.equal(2);
			expect(codecs).to.contain.all.keys([
				EntityType.mosaicRestrictionAddress.toString(),
				EntityType.mosaicRestrictionGlobal.toString()
			]);
		});

		const getCodec = entityType => getCodecs()[entityType];

		describe('supports mosaic restriction address', () => {
			const targetAddress = test.random.bytes(test.constants.sizes.addressDecoded); // 25

			test.binary.test.addAll(getCodec(EntityType.mosaicRestrictionAddress), 57, () => ({
				buffer: Buffer.concat([
					Buffer.of(0xA4, 0x78, 0xB2, 0x05, 0x04, 0x40, 0x38, 0x36), // mosaicId
					Buffer.of(0xFF, 0x12, 0x77, 0x31, 0x82, 0x33, 0x32, 0x29), // restrictionKey
					Buffer.from(targetAddress), // targetAddress 25b
					Buffer.of(0xD3, 0xA1, 0x3E, 0x35, 0x02, 0x22, 0xC5, 0xC4), // previousRestrictionValue
					Buffer.of(0xCC, 0x33, 0xC2, 0x2A, 0x23, 0x32, 0x67, 0xAC) // newRestrictionValue
				]),

				object: {
					mosaicId: [0x05B278A4, 0x36384004],
					restrictionKey: [0x317712FF, 0x29323382],
					targetAddress,
					previousRestrictionValue: [0x353EA1D3, 0xC4C52202],
					newRestrictionValue: [0x2AC233CC, 0xAC673223]
				}
			}));
		});

		describe('supports mosaic restriction global', () => {
			test.binary.test.addAll(getCodec(EntityType.mosaicRestrictionGlobal), 42, () => ({
				buffer: Buffer.concat([
					Buffer.of(0x03, 0xC1, 0xC2, 0x33, 0xB2, 0xFF, 0x23, 0xAC), // mosaicId
					Buffer.of(0x45, 0x32, 0x27, 0xAA, 0x23, 0xC2, 0x2B, 0xEE), // referenceMosaicId
					Buffer.of(0xC4, 0x56, 0x12, 0xB5, 0xF3, 0x3A, 0xA3, 0x01), // restrictionKey
					Buffer.of(0xDD, 0x2E, 0x3C, 0x56, 0x77, 0x7F, 0xF7, 0x7F), // previousRestrictionValue
					Buffer.of(0x01), // previousRestrictionType
					Buffer.of(0x34, 0x03, 0x0F, 0x0C, 0x0C, 0x00, 0x11, 0xB2), // newRestrictionValue
					Buffer.of(0x02) // newRestrictionType
				]),

				object: {
					mosaicId: [0x33C2C103, 0xAC23FFB2],
					referenceMosaicId: [0xAA273245, 0xEE2BC223],
					restrictionKey: [0xB51256C4, 0x01A33AF3],
					previousRestrictionValue: [0x563C2EDD, 0x7FF77F77],
					previousRestrictionType: 0x01,
					newRestrictionValue: [0x0C0F0334, 0xB211000C],
					newRestrictionType: 0x02
				}
			}));
		});
	});
});
