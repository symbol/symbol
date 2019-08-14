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
const lockSecret = require('../../src/plugins/lockSecret');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

describe('lock secret plugin', () => {
	describe('register schema', () => {
		it('adds lock secret system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			lockSecret.registerSchema(builder);
			const modelSchema = builder.build();
			const assertSchema = (schema, expectedSchemaSize, ...expectedKeys) => {
				expect(Object.keys(schema).length).to.equal(expectedSchemaSize);
				expect(schema).to.contain.all.keys(...expectedKeys);
			};

			// Assert:
			assertSchema(modelSchema, numDefaultKeys + 4, [
				'secretLockInfo',
				'secretLockInfo.lock',
				'secretLock',
				'secretProof'
			]);

			// - secret lock infos
			assertSchema(modelSchema['secretLockInfo.lock'], 6,
				'account', 'accountAddress', 'mosaicId', 'height', 'secret', 'recipient');

			// - secret lock transactions
			const transactionSchemaSize = Object.keys(modelSchema.transaction).length;
			assertSchema(modelSchema.secretLock, transactionSchemaSize + 4, 'mosaicId', 'duration', 'secret', 'recipient');
			assertSchema(modelSchema.secretProof, transactionSchemaSize + 3, 'secret', 'recipient', 'proof');
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			lockSecret.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds lock secret codecs', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codecs were registered
			expect(Object.keys(codecs).length).to.equal(2);
			expect(codecs).to.contain.all.keys([
				EntityType.secretLock.toString(),
				EntityType.secretProof.toString()
			]);
		});

		const getCodec = entityType => getCodecs()[entityType];

		describe('supports secret lock', () => {
			const recipientBuffer = test.random.bytes(test.constants.sizes.addressDecoded);
			const secretBuffer = test.random.bytes(test.constants.sizes.hash256);

			test.binary.test.addAll(getCodec(EntityType.secretLock), 74, () => ({
				buffer: Buffer.concat([
					Buffer.of(0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF), // mosaic
					Buffer.of(0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF), // duration
					Buffer.of(0xFF), // hash algorithm
					Buffer.from(secretBuffer), // secret 25b
					Buffer.from(recipientBuffer) // recipient 32b
				]),

				object: {
					mosaic: [0x78563412, 0xEFCDAB90],
					duration: [0xBBAA0099, 0xFFEEDDCC],
					hashAlgorithm: 0xFF,
					secret: secretBuffer,
					recipient: recipientBuffer
				}
			}));
		});

		describe('supports secret proof', () => {
			const Secret_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
			const Recipient_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.addressDecoded));
			const proofBufferSize = 300;
			const Proof_Buffer = Buffer.from(test.random.bytes(proofBufferSize));

			const generateTransaction = () => {
				const data = {
					buffer: Buffer.concat([
						Buffer.of(0xFF), // hash algorithm
						Secret_Buffer, // secret 32b
						Recipient_Buffer, // recipient 25b
						Buffer.of(0x00, 0x00), // proof size 2b
						Proof_Buffer // proofBufferSize
					]),

					object: {
						hashAlgorithm: 0xFF,
						secret: Secret_Buffer,
						recipient: Recipient_Buffer,
						proof: Proof_Buffer
					}
				};
				data.buffer.writeUInt16LE(Proof_Buffer.length, 1 + 32 + 25);
				return data;
			};

			const size = 1 + 32 + 25 + 2 + proofBufferSize;
			test.binary.test.addAll(getCodec(EntityType.secretProof), size, generateTransaction);
		});
	});
});
