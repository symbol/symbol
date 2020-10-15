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
const restrictionsPlugin = require('../../src/plugins/restrictions');
const { AccountRestrictionType } = require('../../src/plugins/restrictions');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

describe('restrictions plugin', () => {
	describe('account restriction types enumeration', () => {
		it('contains valid values', () => {
			const accountRestrictionTypeBlockOffset = 0x8000;

			// Assert:
			expect(AccountRestrictionType.addressAllow).to.equal(0x0001);
			expect(AccountRestrictionType.addressBlock).to.equal(0x0001 + accountRestrictionTypeBlockOffset);
			expect(AccountRestrictionType.mosaicAllow).to.equal(0x0002);
			expect(AccountRestrictionType.mosaicBlock).to.equal(0x0002 + accountRestrictionTypeBlockOffset);
			expect(AccountRestrictionType.operationAllow).to.equal(0x0004);
			expect(AccountRestrictionType.operationBlock).to.equal(0x0004 + accountRestrictionTypeBlockOffset);
		});
	});

	describe('register schema', () => {
		// Arrange:
		const builder = new ModelSchemaBuilder();
		const numDefaultKeys = Object.keys(builder.build()).length;
		const accountRestrictionSchemas = [
			'accountRestriction.addressAccountRestriction',
			'accountRestriction.mosaicAccountRestriction',
			'accountRestriction.operationAccountRestriction'
		];

		// Act:
		restrictionsPlugin.registerSchema(builder);
		const modelSchema = builder.build();

		// Assert:
		it('adds restrictions system schema', () => {
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 15);
			expect(modelSchema).to.contain.all.keys([
				'accountRestrictionAddress',
				'accountRestrictionMosaic',
				'accountRestrictionOperation',
				'accountRestrictions',
				'accountRestriction.restrictions',
				'accountRestriction.fallback',
				'mosaicRestrictions',
				'mosaicRestrictions.entry',
				'mosaicRestrictions.entry.restrictions',
				'mosaicRestrictions.entry.restrictions.restriction'
			].concat(accountRestrictionSchemas));
		});

		it('adds account restrictions schemas', () => {
			// - accountRestrictionAddress
			expect(Object.keys(modelSchema.accountRestrictionAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.accountRestrictionAddress).to.contain.all.keys([
				'restrictionFlags', 'restrictionAdditions', 'restrictionDeletions'
			]);

			// - accountRestrictionMosaic
			expect(Object.keys(modelSchema.accountRestrictionMosaic).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.accountRestrictionMosaic).to.contain.all.keys([
				'restrictionFlags', 'restrictionAdditions', 'restrictionDeletions'
			]);

			// - accountRestrictionOperation
			expect(Object.keys(modelSchema.accountRestrictionOperation).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.accountRestrictionOperation).to.contain.all.keys([
				'restrictionFlags', 'restrictionAdditions', 'restrictionDeletions'
			]);

			// - accountRestrictions
			expect(Object.keys(modelSchema.accountRestrictions).length).to.equal(1);
			expect(modelSchema.accountRestrictions).to.contain.all.keys(['accountRestrictions']);

			// - accountRestriction.restrictions
			expect(Object.keys(modelSchema['accountRestriction.restrictions']).length).to.equal(2);
			expect(modelSchema['accountRestriction.restrictions']).to.contain.all.keys(['address', 'restrictions']);

			// - accountRestriction address, mosaic, and operation restrictions
			accountRestrictionSchemas.forEach(schema => {
				expect(Object.keys(modelSchema[schema]).length).to.equal(2);
				expect(modelSchema[schema]).to.contain.all.keys(['restrictionFlags', 'values']);
			});
		});

		it('uses the correct conditional account schema depending on restriction type', () => {
			// Arrange:
			const accountRestrictionSchema = modelSchema['accountRestriction.restrictions'].restrictions.schemaName;

			// Assert:
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.addressAllow }))
				.to.equal('accountRestriction.addressAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.addressBlock }))
				.to.equal('accountRestriction.addressAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.mosaicAllow }))
				.to.equal('accountRestriction.mosaicAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.mosaicBlock }))
				.to.equal('accountRestriction.mosaicAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.operationAllow }))
				.to.equal('accountRestriction.operationAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: AccountRestrictionType.operationBlock }))
				.to.equal('accountRestriction.operationAccountRestriction');
			expect(accountRestrictionSchema({ restrictionFlags: 99 }))
				.to.equal('accountRestriction.fallback');
		});

		it('adds mosaic restrictions system schemas', () => {
			// - mosaic restriction address transaction
			expect(Object.keys(modelSchema.mosaicRestrictionAddress).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(modelSchema.mosaicRestrictionAddress).to.contain.all.keys([
				'mosaicId',
				'restrictionKey',
				'targetAddress',
				'previousRestrictionValue',
				'newRestrictionValue'
			]);

			// - mosaic restriction global transaction
			expect(Object.keys(modelSchema.mosaicRestrictionGlobal).length).to.equal(Object.keys(modelSchema.transaction).length + 7);
			expect(modelSchema.mosaicRestrictionGlobal).to.contain.all.keys([
				'mosaicId',
				'referenceMosaicId',
				'restrictionKey',
				'previousRestrictionValue',
				'newRestrictionValue',
				'previousRestrictionType',
				'newRestrictionType'
			]);

			// - mosaic restrictions
			expect(Object.keys(modelSchema.mosaicRestrictions).length).to.equal(2);
			expect(modelSchema.mosaicRestrictions).to.contain.all.keys(['id', 'mosaicRestrictionEntry']);

			// - mosaicRestriction.entry
			expect(Object.keys(modelSchema['mosaicRestrictions.entry']).length).to.equal(5);
			expect(modelSchema['mosaicRestrictions.entry']).to.contain.all.keys([
				'compositeHash', 'entryType', 'mosaicId', 'targetAddress', 'restrictions'
			]);

			// - mosaicRestrictions.entry.restrictions
			expect(Object.keys(modelSchema['mosaicRestrictions.entry.restrictions']).length).to.equal(3);
			expect(modelSchema['mosaicRestrictions.entry.restrictions']).to.contain.all.keys([
				'key', 'value', 'restriction'
			]);

			// - mosaicRestrictions.entry.restrictions.restriction
			expect(Object.keys(modelSchema['mosaicRestrictions.entry.restrictions.restriction']).length).to.equal(3);
			expect(modelSchema['mosaicRestrictions.entry.restrictions.restriction']).to.contain.all.keys([
				'referenceMosaicId', 'restrictionValue', 'restrictionType'
			]);
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			restrictionsPlugin.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds restriction codecs (account and mosaic)', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codecs were registered
			expect(Object.keys(codecs).length).to.equal(5);
			expect(codecs).to.contain.all.keys([
				EntityType.accountRestrictionAddress.toString(),
				EntityType.accountRestrictionMosaic.toString(),
				EntityType.accountRestrictionOperation.toString(),
				EntityType.mosaicRestrictionAddress.toString(),
				EntityType.mosaicRestrictionGlobal.toString()
			]);
		});

		describe('account restrictions codecs', () => {
			const addressCodec = getCodecs()[EntityType.accountRestrictionAddress];
			const mosaicCodec = getCodecs()[EntityType.accountRestrictionMosaic];
			const operationCodec = getCodecs()[EntityType.accountRestrictionOperation];

			const generateTransaction = restrictionFlags => {
				const data = {
					buffer: Buffer.concat([
						Buffer.of(0x00, 0x00), // account restriction type 2b
						Buffer.of(0x00), // restrictionAdditions count 1b
						Buffer.of(0x00), // restrictionDeletions count 1b
						Buffer.of(0x00, 0x00, 0x00, 0x00) // account restriction transaction body reserved 1 4b
					]),
					object: {
						restrictionFlags,
						accountRestrictionTransactionBody_Reserved1: 0,
						restrictionAdditions: [],
						restrictionDeletions: []
					}
				};
				data.buffer.writeUInt32LE(restrictionFlags);

				return data;
			};

			const addNoModificationTests = (restrictionFlags, codec) => {
				test.binary.test.addAll(codec, 8, () => generateTransaction(restrictionFlags));
			};

			const addModificationTests = (restrictionType, codec, additionsBuffer, additionsValues, deletionsBuffer, deletionsValues) => {
				const data = generateTransaction(restrictionType);

				data.buffer = Buffer.concat([data.buffer, additionsBuffer, deletionsBuffer]);

				data.buffer.writeUInt8(additionsValues.length, 2); // restrictionAdditionsCount, additions at 2 bytes offset
				data.buffer.writeUInt8(deletionsValues.length, 3); // restrictionDeletionsCount, deletions at 3 bytes offset

				data.object.restrictionAdditions = additionsValues;
				data.object.restrictionDeletions = deletionsValues;

				const bufferSize = 8 + additionsBuffer.length + deletionsBuffer.length;

				test.binary.test.addAll(codec, bufferSize, () => (data));
			};

			describe('supports account address restriction with no additions or deletions', () => {
				addNoModificationTests(AccountRestrictionType.addressAllow, addressCodec);
			});

			describe('supports account mosaic restriction with no additions or deletions', () => {
				addNoModificationTests(AccountRestrictionType.mosaicAllow, mosaicCodec);
			});

			describe('supports account operation restriction with no additions or deletions', () => {
				addNoModificationTests(AccountRestrictionType.operationAllow, operationCodec);
			});

			describe('supports account address restriction with one addition and deletion', () => {
				const additionTestAddress = test.random.bytes(test.constants.sizes.addressDecoded);
				const deletionTestAddress = test.random.bytes(test.constants.sizes.addressDecoded);

				addModificationTests(
					AccountRestrictionType.addressAllow,
					addressCodec,
					Buffer.from(additionTestAddress),
					[additionTestAddress],
					Buffer.from(deletionTestAddress),
					[deletionTestAddress]
				);
			});

			describe('supports account mosaic restriction with one addition and deletion', () => {
				addModificationTests(
					AccountRestrictionType.mosaicAllow,
					mosaicCodec,
					Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x92),
					[[0x066C26F2, 0x92B28340]],
					Buffer.of(0xE3, 0x12, 0x4B, 0x33, 0x25, 0x38, 0x00, 0x12),
					[[0x334B12E3, 0x12003825]]
				);
			});

			describe('supports account operation restriction with one addition and deletion', () => {
				addModificationTests(
					AccountRestrictionType.operationAllow,
					operationCodec,
					Buffer.of(0xF2, 0x83),
					[0x83F2],
					Buffer.of(0x03, 0x44),
					[0x4403]
				);
			});

			describe('supports account address restriction with several additions and deletions', () => {
				const testAddress1 = test.random.bytes(test.constants.sizes.addressDecoded);
				const testAddress2 = test.random.bytes(test.constants.sizes.addressDecoded);
				const testAddress3 = test.random.bytes(test.constants.sizes.addressDecoded);
				const testAddress4 = test.random.bytes(test.constants.sizes.addressDecoded);
				const testAddress5 = test.random.bytes(test.constants.sizes.addressDecoded);

				addModificationTests(
					AccountRestrictionType.addressAllow,
					addressCodec,
					Buffer.concat([Buffer.from(testAddress1), Buffer.from(testAddress2), Buffer.from(testAddress3)]),
					[testAddress1, testAddress2, testAddress3],
					Buffer.concat([Buffer.from(testAddress4), Buffer.from(testAddress5)]),
					[testAddress4, testAddress5]
				);
			});

			describe('supports account mosaic restriction with several additions and deletions', () => {
				const mosaic1 = Buffer.of(0xF2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x10);
				const mosaic2 = Buffer.of(0xB2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x11);
				const mosaic3 = Buffer.of(0xD2, 0x26, 0x6C, 0x06, 0x40, 0x83, 0xB2, 0x12);

				const mosaic4 = Buffer.of(0xAA, 0x00, 0x4B, 0x33, 0x25, 0x38, 0x00, 0x13);
				const mosaic5 = Buffer.of(0xBB, 0x00, 0x4B, 0x33, 0x25, 0x38, 0x00, 0x14);

				addModificationTests(
					AccountRestrictionType.mosaicAllow,
					mosaicCodec,
					Buffer.concat([mosaic1, mosaic2, mosaic3]),
					[[0x066C26F2, 0x10B28340], [0x066C26B2, 0x11B28340], [0x066C26D2, 0x12B28340]],
					Buffer.concat([mosaic4, mosaic5]),
					[[0x334B00AA, 0x13003825], [0x334B00BB, 0x14003825]]
				);
			});

			describe('supports account operation restriction with several additions and deletions', () => {
				addModificationTests(
					AccountRestrictionType.operationAllow,
					operationCodec,
					Buffer.concat([Buffer.of(0xF2, 0x83), Buffer.of(0xAA, 0xBB), Buffer.of(0x01, 0x10)]),
					[0x83F2, 0xBBAA, 0x1001],
					Buffer.concat([Buffer.of(0x03, 0x44), Buffer.of(0x12, 0x34)]),
					[0x4403, 0x3412]
				);
			});
		});

		describe('mosaic restrictions codecs', () => {
			const getCodec = entityType => getCodecs()[entityType];

			describe('supports mosaic restriction address', () => {
				const targetAddress = test.random.bytes(test.constants.sizes.addressDecoded); // 24

				test.binary.test.addAll(getCodec(EntityType.mosaicRestrictionAddress), 56, () => ({
					buffer: Buffer.concat([
						Buffer.of(0xA4, 0x78, 0xB2, 0x05, 0x04, 0x40, 0x38, 0x36), // mosaicId
						Buffer.of(0xFF, 0x12, 0x77, 0x31, 0x82, 0x33, 0x32, 0x29), // restrictionKey
						Buffer.of(0xD3, 0xA1, 0x3E, 0x35, 0x02, 0x22, 0xC5, 0xC4), // previousRestrictionValue
						Buffer.of(0xCC, 0x33, 0xC2, 0x2A, 0x23, 0x32, 0x67, 0xAC), // newRestrictionValue
						Buffer.from(targetAddress) // targetAddress 24b
					]),

					object: {
						mosaicId: [0x05B278A4, 0x36384004],
						restrictionKey: [0x317712FF, 0x29323382],
						previousRestrictionValue: [0x353EA1D3, 0xC4C52202],
						newRestrictionValue: [0x2AC233CC, 0xAC673223],
						targetAddress
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
						Buffer.of(0x34, 0x03, 0x0F, 0x0C, 0x0C, 0x00, 0x11, 0xB2), // newRestrictionValue
						Buffer.of(0x01), // previousRestrictionType
						Buffer.of(0x02) // newRestrictionType
					]),

					object: {
						mosaicId: [0x33C2C103, 0xAC23FFB2],
						referenceMosaicId: [0xAA273245, 0xEE2BC223],
						restrictionKey: [0xB51256C4, 0x01A33AF3],
						previousRestrictionValue: [0x563C2EDD, 0x7FF77F77],
						newRestrictionValue: [0x0C0F0334, 0xB211000C],
						previousRestrictionType: 0x01,
						newRestrictionType: 0x02
					}
				}));
			});
		});
	});
});
