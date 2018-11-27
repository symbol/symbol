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
const test = require('../binaryTestUtils');
const transfer = require('../../src/plugins/transfer');
const { expect } = require('chai');

const constants = {
	sizes: {
		transfer: 28,
		message: 0x70,
		mosaics: 0x50
	}
};

describe('transfer plugin', () => {
	describe('register schema', () => {
		it('adds transfer system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			transfer.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 2);
			expect(modelSchema).to.contain.all.keys(['transfer', 'transfer.message']);

			// - transfer
			expect(Object.keys(modelSchema.transfer).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.transfer).to.contain.all.keys(['recipient', 'message', 'mosaics']);

			// - message
			expect(modelSchema['transfer.message']).to.deep.equal({
				payload: ModelType.binary
			});
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			transfer.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds transfer codec', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codec was registered
			expect(Object.keys(codecs).length).to.equal(1);
			expect(codecs).to.contain.all.keys([EntityType.transfer.toString()]);
		});

		const generateTransaction = () => {
			const Recipient_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.addressDecoded));

			return {
				buffer: Buffer.concat([
					Recipient_Buffer,
					Buffer.of(0x00, 0x00, 0x00) // footer
				]),

				object: {
					recipient: Recipient_Buffer
				}
			};
		};

		const addMessage = generator => {
			const Message_Buffer = Buffer.from([
				0x99, 0xF2, 0x26, 0x6C, 0x06, 0xBE, 0xE0, 0xE1, 0xC7, 0x39, 0x57, 0xFE, 0x0F, 0x39, 0x7E, 0x7A,
				0xE3, 0x15, 0xEA, 0x51, 0x6B, 0xA7, 0x12, 0xEF, 0x82, 0x7C, 0xE6, 0x2B, 0xD9, 0x5E, 0x01, 0xEC,
				0x31, 0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA,
				0x47, 0x0D, 0xB8, 0xFD, 0x2D, 0x81, 0x47, 0x6A, 0xC5, 0x61, 0xA4, 0xCE, 0xE1, 0x81, 0x40, 0x83,
				0x20, 0x3E, 0xCA, 0x9E, 0x17, 0x1A, 0x02, 0xFB, 0xD4, 0x9C, 0x73, 0x75, 0x5D, 0x82, 0xEE, 0xCE,
				0x63, 0x90, 0x5A, 0x44, 0xA2, 0x7C, 0xF1, 0x3A, 0x7B, 0x77, 0xA9, 0xB3, 0x8A, 0xD1, 0xB2, 0x92,
				0x86, 0xFF, 0x21, 0x2F, 0x49, 0x7A, 0x34, 0x14, 0xC9, 0x88, 0xE7, 0x79, 0x6A, 0x6F, 0xC9
			]);

			return () => {
				const data = generator();
				data.buffer = Buffer.concat([
					data.buffer,
					Buffer.of(0x90), // message type
					Message_Buffer
				]);
				data.buffer.writeUInt16LE(constants.sizes.message, constants.sizes.transfer - 3);

				data.object.message = { type: 0x90, payload: Buffer.from(Message_Buffer) };
				return data;
			};
		};

		const addMessageWithTypeOnly = generator => () => {
			const data = generator();
			data.buffer = Buffer.concat([
				data.buffer,
				Buffer.of(0x90) // message type
			]);
			data.buffer.writeUInt16LE(1, constants.sizes.transfer - 3);

			data.object.message = { type: 0x90, payload: [] };
			return data;
		};

		const addMosaics = generator => {
			const Mosaics_Buffer = Buffer.from([
				0xED, 0x3E, 0x8A, 0xAD, 0xEC, 0xAD, 0xDA, 0x3F, 0x33, 0x05, 0x49, 0x3C, 0x6C, 0x97, 0xAE, 0x94,
				0xA3, 0x00, 0xEA, 0xFE, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
				0xB4, 0x64, 0x72, 0x42, 0xF1, 0xFF, 0x11, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0x87, 0xF8,
				0x60, 0xD7, 0xFE, 0xCD, 0xC6, 0x44, 0xD7, 0x1A, 0x6B, 0xAD, 0x02, 0xA4, 0x55, 0x96, 0x0C, 0x00,
				0x12, 0xE0, 0x2D, 0x82, 0x8E, 0x54, 0x86, 0xF3, 0x03, 0x81, 0x70, 0xA9, 0xFE, 0x33, 0x12, 0x2F
			]);

			return () => {
				const data = generator();
				data.buffer = Buffer.concat([
					data.buffer,
					Mosaics_Buffer
				]);
				data.buffer.writeUInt8(5, constants.sizes.transfer - 1);

				data.object.mosaics = [
					{ id: [0xAD8A3EED, 0x3FDAADEC], amount: [0x3C490533, 0x94AE976C] },
					{ id: [0xFEEA00A3, 0xFA5CBDDA], amount: [0x1D944B0D, 0xB151BB15] },
					{ id: [0x427264B4, 0x0011FFF1], amount: [0x8F9AD09F, 0xF887353D] },
					{ id: [0xCDFED760, 0x1AD744C6], amount: [0xA402AD6B, 0x000C9655] },
					{ id: [0x822DE012, 0xF386548E], amount: [0xA9708103, 0x2F1233FE] }
				];
				return data;
			};
		};

		const getCodec = () => getCodecs()[EntityType.transfer];

		describe('supports transfer', () => {
			describe('with neither message nor mosaics', () => {
				test.binary.test.addAll(getCodec(), constants.sizes.transfer, generateTransaction);
			});

			describe('with message', () => {
				test.binary.test.addAll(getCodec(), constants.sizes.transfer + constants.sizes.message, addMessage(generateTransaction));
			});

			describe('with message composed of only type', () => {
				test.binary.test.addAll(getCodec(), constants.sizes.transfer + 1, addMessageWithTypeOnly(generateTransaction));
			});

			describe('with mosaics', () => {
				test.binary.test.addAll(getCodec(), constants.sizes.transfer + constants.sizes.mosaics, addMosaics(generateTransaction));
			});

			describe('with message and mosaics', () => {
				test.binary.test.addAll(
					getCodec(),
					constants.sizes.transfer + constants.sizes.message + constants.sizes.mosaics,
					addMosaics(addMessage(generateTransaction))
				);
			});
		});
	});
});
