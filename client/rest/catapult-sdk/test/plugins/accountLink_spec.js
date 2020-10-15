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
const accountLinkPlugin = require('../../src/plugins/accountLink');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

describe('account link plugin', () => {
	describe('register schema', () => {
		it('adds account link system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			accountLinkPlugin.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 4);
			expect(modelSchema).to.contain.all.keys(['accountLink', 'nodeKeyLink', 'votingKeyLink', 'vrfKeyLink']);

			// - accountLink
			expect(Object.keys(modelSchema.accountLink).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.accountLink).to.contain.all.keys(['linkedPublicKey', 'linkAction']);

			// - nodeKeyLink
			expect(Object.keys(modelSchema.nodeKeyLink).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.nodeKeyLink).to.contain.all.keys(['linkedPublicKey', 'linkAction']);

			// - votingKeyLink
			expect(Object.keys(modelSchema.votingKeyLink).length).to.equal(Object.keys(modelSchema.transaction).length + 4);
			expect(modelSchema.votingKeyLink).to.contain.all.keys(['linkedPublicKey', 'startEpoch', 'endEpoch', 'linkAction']);

			// - vrfKeyLink
			expect(Object.keys(modelSchema.vrfKeyLink).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.vrfKeyLink).to.contain.all.keys(['linkedPublicKey', 'linkAction']);
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			accountLinkPlugin.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds account link codec', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codec was registered
			expect(Object.keys(codecs).length).to.equal(4);
			expect(codecs).to.contain.all.keys([EntityType.accountLink.toString()]);
			expect(codecs).to.contain.all.keys([EntityType.nodeKeyLink.toString()]);
			expect(codecs).to.contain.all.keys([EntityType.votingKeyLink.toString()]);
			expect(codecs).to.contain.all.keys([EntityType.vrfKeyLink.toString()]);
		});

		describe('supports account link transaction', () => {
			const linkedPublicKey = Buffer.of(
				0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA, 0xC9,
				0x47, 0x0D, 0xB8, 0xFD, 0x2D, 0x81, 0x47, 0x6A, 0xC5, 0x61, 0xA4, 0xCE, 0xE1, 0x81, 0x40, 0x83
			);
			test.binary.test.addAll(getCodecs()[EntityType.accountLink], 32 + 1, () => ({
				buffer: Buffer.concat([
					linkedPublicKey,
					Buffer.of(0x01)
				]),
				object: {
					linkedPublicKey,
					linkAction: 0x01
				}
			}));
		});

		describe('supports node key link transaction', () => {
			const linkedPublicKey = Buffer.of(
				0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA, 0xC9,
				0x47, 0x0D, 0xB8, 0xFD, 0x2D, 0x81, 0x47, 0x6A, 0xC5, 0x61, 0xA4, 0xCE, 0xE1, 0x81, 0x40, 0x83
			);
			test.binary.test.addAll(getCodecs()[EntityType.nodeKeyLink], 32 + 1, () => ({
				buffer: Buffer.concat([
					linkedPublicKey,
					Buffer.of(0x01)
				]),
				object: {
					linkedPublicKey,
					linkAction: 0x01
				}
			}));
		});

		describe('supports voting key link transaction', () => {
			const linkedPublicKey = Buffer.of(
				0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA, 0xC9,
				0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA, 0xC9,
				0x47, 0x0D, 0xB8, 0xFD, 0x2D, 0x81, 0x47, 0x6A, 0xC5, 0x61, 0xA4, 0xCE, 0xE1, 0x81, 0x40, 0x83
			);
			const startEpoch = Buffer.of(0x47, 0x12, 0xC3, 0x00);
			const endEpoch = Buffer.of(0x73, 0xBE, 0xD2, 0x11);

			test.binary.test.addAll(getCodecs()[EntityType.votingKeyLink], 48 + 4 + 4 + 1, () => ({
				buffer: Buffer.concat([
					linkedPublicKey, // 48b
					startEpoch, // 4b
					endEpoch, // 4b
					Buffer.of(0x01) // 1b
				]),
				object: {
					linkedPublicKey,
					startEpoch: 0x00C31247,
					endEpoch: 0x11D2BE73,
					linkAction: 0x01
				}
			}));
		});

		describe('supports vrf key link transaction', () => {
			const linkedPublicKey = Buffer.of(
				0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA, 0xC9,
				0x47, 0x0D, 0xB8, 0xFD, 0x2D, 0x81, 0x47, 0x6A, 0xC5, 0x61, 0xA4, 0xCE, 0xE1, 0x81, 0x40, 0x83
			);
			test.binary.test.addAll(getCodecs()[EntityType.vrfKeyLink], 32 + 1, () => ({
				buffer: Buffer.concat([
					linkedPublicKey,
					Buffer.of(0x01)
				]),
				object: {
					linkedPublicKey,
					linkAction: 0x01
				}
			}));
		});
	});
});
