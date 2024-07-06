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

import ModelSchemaBuilder from '../../../src/catapult-sdk/model/ModelSchemaBuilder.js';
import lockHash from '../../../src/catapult-sdk/plugins/lockHash.js';
import { expect } from 'chai';

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
				'TransactionType.HASH_LOCK',
				'hashLockInfo',
				'hashLockInfo.lock'
			]);

			// - TransactionType.HASH_LOCK
			const hashLockSchema = modelSchema['TransactionType.HASH_LOCK'];
			const transactionSchemaSize = Object.keys(modelSchema.transaction).length;
			assertSchema(hashLockSchema, transactionSchemaSize + 4, 'duration', 'hash', 'mosaicId', 'amount');

			// - hash lock infos
			assertSchema(modelSchema.hashLockInfo, 2, 'id', 'lock');
			assertSchema(
				modelSchema['hashLockInfo.lock'], 7,
				'version', 'ownerAddress', 'mosaicId', 'amount', 'endHeight', 'status', 'hash'
			);
		});
	});
});
