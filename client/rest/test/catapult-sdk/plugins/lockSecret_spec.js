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
import lockSecret from '../../../src/catapult-sdk/plugins/lockSecret.js';
import { expect } from 'chai';

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
				'TransactionType.SECRET_LOCK',
				'TransactionType.SECRET_PROOF',
				'secretLockInfo',
				'secretLockInfo.lock'
			]);

			// - TransactionType.SECRET_LOCK
			const transactionSchemaSize = Object.keys(modelSchema.transaction).length;
			const secretLockSchema = modelSchema['TransactionType.SECRET_LOCK'];
			assertSchema(
				secretLockSchema, transactionSchemaSize + 6,
				'secret', 'mosaicId', 'amount', 'duration', 'recipientAddress', 'hashAlgorithm'
			);

			// - TransactionType.SECRET_PROOF
			const secretProofSchema = modelSchema['TransactionType.SECRET_PROOF'];
			assertSchema(secretProofSchema, transactionSchemaSize + 4, 'secret', 'recipientAddress', 'proof', 'hashAlgorithm');

			// - secret lock
			assertSchema(modelSchema.secretLockInfo, 2, 'id', 'lock');

			// - secret lock infos
			assertSchema(
				modelSchema['secretLockInfo.lock'], 10,
				'version', 'ownerAddress', 'mosaicId', 'amount', 'endHeight', 'secret',
				'status', 'hashAlgorithm', 'recipientAddress', 'compositeHash'
			);
		});
	});
});
