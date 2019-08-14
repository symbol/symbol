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
const lockHash = require('../../src/plugins/lockHash');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

describe('lock hash plugin', () => {
	describe('register schema', () => {
		it('adds lock hash system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			lockHash.registerSchema(builder);
			const modelSchema = builder.build();
			const assertSchema = (schema, expectedSchemaSize, ...expectedKeys) => {
				expect(Object.keys(schema).length).to.equal(expectedSchemaSize);
				expect(schema).to.contain.all.keys(...expectedKeys);
			};

			// Assert:
			assertSchema(modelSchema, numDefaultKeys + 3, [
				'hashLockInfo',
				'hashLockInfo.lock',
				'hashLock'
			]);

			// - hash lock infos
			assertSchema(modelSchema.hashLockInfo, 1, 'lock');
			assertSchema(modelSchema['hashLockInfo.lock'], 5, 'account', 'accountAddress', 'mosaicId', 'height', 'hash');

			// - hash lock transaction
			const transactionSchemaSize = Object.keys(modelSchema.transaction).length;
			assertSchema(modelSchema.hashLock, transactionSchemaSize + 3, 'mosaic', 'duration', 'hash');
		});
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			lockHash.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds lock hash codecs', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codecs were registered
			expect(Object.keys(codecs).length).to.equal(1);
			expect(codecs).to.contain.all.keys([EntityType.hashLock.toString()]);
		});

		const getCodec = entityType => getCodecs()[entityType];

		describe('supports hash lock', () => {
			const hash = test.random.bytes(test.constants.sizes.hash256);

			test.binary.test.addAll(getCodec(EntityType.hashLock), 48, () => ({
				buffer: Buffer.concat([
					Buffer.of(0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF), // mosaic
					Buffer.of(0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF), // duration
					Buffer.from(hash) // hash 32b
				]),
				object: {
					mosaic: [0x78563412, 0xEFCDAB90],
					duration: [0xBBAA0099, 0xFFEEDDCC],
					hash
				}
			}));
		});
	});
});
