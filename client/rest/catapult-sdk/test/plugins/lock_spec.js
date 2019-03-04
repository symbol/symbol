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
const lock = require('../../src/plugins/lock');
const ModelSchemaBuilder = require('../../src/model/ModelSchemaBuilder');
const sizes = require('../../src/modelBinary/sizes');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

const constants = {
	sizes: {
		hashLockSize: 24 + 32,
		secretLockSize: 24 + 1 + 32 + 25,
		secretProof: 1 + 32 + 2
	}
};

Object.assign(constants.sizes, sizes);

describe('lock plugin', () => {
	describe('register schema', () => {
		it('adds lock system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			lock.registerSchema(builder);
			const modelSchema = builder.build();
			const assertSchema = (schema, expectedSchemaSize, ...expectedKeys) => {
				expect(Object.keys(schema).length).to.equal(expectedSchemaSize);
				expect(schema).to.contain.all.keys(...expectedKeys);
			};

			// Assert:
			assertSchema(modelSchema, numDefaultKeys + 7, [
				'hashLockInfo',
				'hashLockInfo.lock',
				'secretLockInfo',
				'secretLockInfo.lock',
				'hashLock',
				'secretLock',
				'secretProof'
			]);

			// - lock infos
			assertSchema(modelSchema.hashLockInfo, 1, 'lock');
			assertSchema(modelSchema['hashLockInfo.lock'], 6, 'account', 'accountAddress', 'mosaicId', 'amount', 'height', 'hash');

			assertSchema(modelSchema.secretLockInfo, 1, 'lock');
			assertSchema(
				modelSchema['secretLockInfo.lock'],
				7,
				'account', 'accountAddress', 'mosaicId', 'amount', 'height', 'secret', 'recipient'
			);

			// - transactions
			const transactionSchemaSize = Object.keys(modelSchema.transaction).length;
			assertSchema(modelSchema.hashLock, transactionSchemaSize + 4, 'mosaicId', 'amount', 'duration', 'hash');
			assertSchema(modelSchema.secretLock, transactionSchemaSize + 5, 'mosaicId', 'amount', 'duration', 'secret', 'recipient');
			assertSchema(modelSchema.secretProof, transactionSchemaSize + 2, 'secret', 'proof');
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			lock.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds lock codecs', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codecs were registered
			expect(Object.keys(codecs).length).to.equal(3);
			expect(codecs).to.contain.all.keys([
				EntityType.hashLock.toString(),
				EntityType.secretLock.toString(),
				EntityType.secretProof.toString()
			]);
		});

		const getCodec = entityType => getCodecs()[entityType];

		const generateLockTransaction = () => ({
			buffer: Buffer.concat([
				Buffer.of(0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF), // mosaicId
				Buffer.of(0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88), // amount
				Buffer.of(0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF) // duration
			]),

			object: {
				mosaicId: [0x78563412, 0xEFCDAB90],
				amount: [0x44332211, 0x88776655],
				duration: [0xBBAA0099, 0xFFEEDDCC]
			}
		});

		describe('supports hash lock', () => {
			const Hash_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));

			const addHashLockProperties = generator => () => {
				const data = generator();
				data.buffer = Buffer.concat([
					data.buffer,
					Hash_Buffer
				]);

				data.object.hash = Hash_Buffer;
				return data;
			};

			const size = constants.sizes.hashLockSize;
			test.binary.test.addAll(getCodec(EntityType.hashLock), size, addHashLockProperties(generateLockTransaction));
		});

		describe('supports secret lock', () => {
			const Recipient_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.addressDecoded));
			const Secret_Buffer = Buffer.from(test.random.bytes(constants.sizes.hash256));

			const addSecretLockProperties = generator => () => {
				const data = generator();
				data.buffer = Buffer.concat([
					data.buffer,
					Buffer.of(0xFF), // hash algorithm
					Secret_Buffer, // secret
					Recipient_Buffer // recipient
				]);

				data.object.hashAlgorithm = 0xFF;
				data.object.secret = Secret_Buffer;
				data.object.recipient = Recipient_Buffer;
				return data;
			};

			const size = constants.sizes.secretLockSize;
			test.binary.test.addAll(getCodec(EntityType.secretLock), size, addSecretLockProperties(generateLockTransaction));
		});

		describe('supports secret proof', () => {
			const Secret_Buffer = Buffer.from(test.random.bytes(constants.sizes.hash256));
			const Proof_Buffer = Buffer.from(test.random.bytes(300));

			const generateTransaction = () => {
				const data = {
					buffer: Buffer.concat([
						Buffer.of(0xFF), // hash algorithm
						Secret_Buffer, // secret
						Buffer.of(0x00, 0x00), // proof size
						Proof_Buffer
					]),

					object: {
						hashAlgorithm: 0xFF,
						secret: Secret_Buffer,
						proof: Proof_Buffer
					}
				};
				data.buffer.writeUInt16LE(Proof_Buffer.length, constants.sizes.secretProof - 2);
				return data;
			};

			const size = constants.sizes.secretProof + Proof_Buffer.length;
			test.binary.test.addAll(getCodec(EntityType.secretProof), size, generateTransaction);
		});
	});
});
