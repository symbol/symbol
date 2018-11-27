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

const accountLinkPlugin = require('../../src/plugins/accountLink');

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
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 1);
			expect(modelSchema).to.contain.all.keys(['accountLink']);

			// - accountLink
			expect(Object.keys(modelSchema.accountLink).length).to.equal(Object.keys(modelSchema.transaction).length + 1);
			expect(modelSchema.accountLink).to.contain.all.keys(['remoteAccountKey']);
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
			expect(Object.keys(codecs).length).to.equal(1);
			expect(codecs).to.contain.all.keys([EntityType.accountLink.toString()]);
		});

		const codec = getCodecs()[EntityType.accountLink];

		describe('supports account link transaction', () => {
			const remoteAccountKey = Buffer.of(
				0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA, 0xC9,
				0x47, 0x0D, 0xB8, 0xFD, 0x2D, 0x81, 0x47, 0x6A, 0xC5, 0x61, 0xA4, 0xCE, 0xE1, 0x81, 0x40, 0x83
			);
			test.binary.test.addAll(codec, 32 + 1, () => ({
				buffer: Buffer.concat([
					remoteAccountKey,
					Buffer.of(0x01)
				]),
				object: {
					remoteAccountKey,
					linkAction: 0x01
				}
			}));
		});
	});
});
