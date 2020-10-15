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
const multisig = require('../../src/plugins/multisig');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

const constants = {
	sizes: {
		addressDecoded: 24,
		modifyMultisigAccount: 8
	}
};

describe('multisig plugin', () => {
	describe('register schema', () => {
		it('adds multisig system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			multisig.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 4);
			expect(modelSchema).to.contain.all.keys([
				'modifyMultisigAccount',
				'multisigEntry',
				'multisigEntry.multisig',
				'multisigGraph'
			]);

			// - modify multisig account
			expect(Object.keys(modelSchema.modifyMultisigAccount).length).to.equal(Object.keys(modelSchema.transaction).length + 4);
			expect(modelSchema.modifyMultisigAccount).to.contain.all.keys(
				['minRemovalDelta', 'minApprovalDelta', 'addressAdditions', 'addressDeletions']
			);

			// - multisig entry
			expect(Object.keys(modelSchema.multisigEntry).length).to.equal(1);
			expect(modelSchema.multisigEntry).to.contain.all.keys(['multisig']);

			expect(Object.keys(modelSchema['multisigEntry.multisig']).length).to.equal(5);
			expect(modelSchema['multisigEntry.multisig'])
				.to.contain.all.keys(['accountAddress', 'minApproval', 'minRemoval', 'multisigAddresses', 'cosignatoryAddresses']);

			// - multisig graph
			expect(Object.keys(modelSchema.multisigGraph).length).to.equal(2);
			expect(modelSchema.multisigGraph).to.contain.all.keys(['level', 'multisigEntries']);
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			multisig.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds multisig codec', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codec was registered
			expect(Object.keys(codecs).length).to.equal(1);
			expect(codecs).to.contain.all.keys([EntityType.modifyMultisigAccount.toString()]);
		});

		const generateTransaction = () => ({
			buffer: Buffer.concat([
				Buffer.of(0x2B), // minRemovalDelta 1b
				Buffer.of(0x4D), // minApprovalDelta 1b
				Buffer.of(0x00), // addressAdditionsCount 1b
				Buffer.of(0x00), // addressDeletionsCount 1b
				Buffer.of(0x00, 0x00, 0x00, 0x00) // multisig account modification transaction body reserved 1 4b
			]),
			object: {
				minRemovalDelta: 0x2B,
				minApprovalDelta: 0x4D,
				multisigAccountModificationTransactionBody_Reserved1: 0,
				addressAdditions: [],
				addressDeletions: []
			}
		});

		const addModifications = generator => {
			const addressAddition1 = Buffer.of(
				0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA, 0xC9,
				0x47, 0x0D, 0xB8, 0xFD, 0x2D, 0x81, 0x47, 0x6A
			);
			const addressAddition2 = Buffer.of(
				0x3E, 0xCA, 0x9E, 0x17, 0x1A, 0x02, 0xFB, 0xD4, 0x9C, 0x73, 0x75, 0x5D, 0x82, 0xEE, 0xCE, 0x6F,
				0x63, 0x90, 0x5A, 0x44, 0xA2, 0x7C, 0xF1, 0x3A
			);
			const addressDeletion1 = Buffer.of(
				0x99, 0xF2, 0x26, 0x6C, 0x06, 0xBE, 0xE0, 0xE1, 0xC7, 0x39, 0x57, 0xFE, 0x0F, 0x39, 0x7E, 0x7A,
				0xE3, 0x15, 0xEA, 0x51, 0x6B, 0xA7, 0x12, 0xEF
			);

			return () => {
				const data = generator();
				data.buffer = Buffer.concat([data.buffer, addressAddition1, addressAddition2, addressDeletion1]);

				data.buffer.writeUInt8(2, 2); // addressAdditionsCount, two additions at 2 bytes offset
				data.buffer.writeUInt8(1, 3); // addressDeletionsCount, one deletion at 3 bytes offset

				data.object.addressAdditions = [addressAddition1, addressAddition2];
				data.object.addressDeletions = [addressDeletion1];

				return data;
			};
		};

		const getCodec = () => getCodecs()[EntityType.modifyMultisigAccount];

		describe('supports modify multisig account', () => {
			describe('with no additions or deletions', () => {
				test.binary.test.addAll(getCodec(), constants.sizes.modifyMultisigAccount, generateTransaction);
			});

			describe('with no additions or deletions and negative deltas', () => {
				test.binary.test.addAll(getCodec(), constants.sizes.modifyMultisigAccount, () => ({
					buffer: Buffer.concat([
						Buffer.of(0xA2), // minRemovalDelta 1b
						Buffer.of(0xC9), // minApprovalDelta 1b
						Buffer.of(0x00), // addressAdditionsCount 1b
						Buffer.of(0x00), // addressDeletionsCount 1b
						Buffer.of(0x00, 0x00, 0x00, 0x00) // multisig account modification transaction body reserved 1 4b
					]),
					object: {
						minRemovalDelta: -94,
						minApprovalDelta: -55,
						addressAdditions: [],
						addressDeletions: [],
						multisigAccountModificationTransactionBody_Reserved1: 0
					}
				}));
			});

			describe('with additions and deletions', () => {
				test.binary.test.addAll(
					getCodec(),
					constants.sizes.modifyMultisigAccount + (3 * constants.sizes.addressDecoded),
					addModifications(generateTransaction)
				);
			});
		});
	});
});
