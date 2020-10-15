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

			// - secret lock
			assertSchema(modelSchema.secretLockInfo, 2, 'id', 'lock');

			// - secret lock infos
			assertSchema(modelSchema['secretLockInfo.lock'], 9,
				'ownerAddress', 'mosaicId', 'amount', 'endHeight', 'secret',
				'status', 'hashAlgorithm', 'recipientAddress', 'compositeHash');

			// - secret lock transactions
			const transactionSchemaSize = Object.keys(modelSchema.transaction).length;
			assertSchema(modelSchema.secretLock, transactionSchemaSize + 6,
				'secret', 'mosaicId', 'amount', 'duration', 'recipientAddress', 'hashAlgorithm');
			assertSchema(modelSchema.secretProof, transactionSchemaSize + 4, 'secret', 'recipientAddress', 'proof', 'hashAlgorithm');
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
			const recipientAddressBuffer = test.random.bytes(test.constants.sizes.addressDecoded);
			const secretBuffer = test.random.bytes(test.constants.sizes.hash256);

			test.binary.test.addAll(getCodec(EntityType.secretLock), 81, () => ({
				buffer: Buffer.concat([
					Buffer.from(recipientAddressBuffer), // recipientAddress 24b
					Buffer.from(secretBuffer), // secret 32b
					Buffer.of(0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF), // mosaic
					Buffer.of(0xCA, 0xD0, 0x8E, 0x6E, 0xFF, 0x21, 0x2F, 0x49), // amount
					Buffer.of(0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF), // duration
					Buffer.of(0xFF) // hash algorithm
				]),

				object: {
					recipientAddress: recipientAddressBuffer,
					secret: secretBuffer,
					mosaicId: [0x78563412, 0xEFCDAB90],
					amount: [0x6E8ED0CA, 0x492F21FF],
					duration: [0xBBAA0099, 0xFFEEDDCC],
					hashAlgorithm: 0xFF
				}
			}));
		});

		describe('supports secret proof', () => {
			const RecipientAddress_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.addressDecoded));
			const Secret_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
			const proofBufferSize = 300;
			const Proof_Buffer = Buffer.from(test.random.bytes(proofBufferSize));

			const generateTransaction = () => {
				const data = {
					buffer: Buffer.concat([
						RecipientAddress_Buffer, // recipient 24b
						Secret_Buffer, // secret 32b
						Buffer.of(0x00, 0x00), // proof size 2b
						Buffer.of(0xFF), // hash algorithm
						Proof_Buffer // proofBufferSize
					]),

					object: {
						recipientAddress: RecipientAddress_Buffer,
						secret: Secret_Buffer,
						hashAlgorithm: 0xFF,
						proof: Proof_Buffer
					}
				};
				data.buffer.writeUInt16LE(Proof_Buffer.length, 24 + 32);
				return data;
			};

			const size = test.constants.sizes.hash256 + 2 + 1 + test.constants.sizes.addressDecoded + proofBufferSize;
			test.binary.test.addAll(getCodec(EntityType.secretProof), size, generateTransaction);
		});
	});
});
