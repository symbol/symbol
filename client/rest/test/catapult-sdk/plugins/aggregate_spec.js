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
import ModelType from '../../../src/catapult-sdk/model/ModelType.js';
import aggregate from '../../../src/catapult-sdk/plugins/aggregate.js';
import { expect } from 'chai';

describe('aggregate plugin', () => {
	describe('register schema', () => {
		const assertAddsSchema = schemaName => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			aggregate.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 3);
			expect(modelSchema).to.contain.all.keys([
				'TransactionType.AGGREGATE_COMPLETE',
				'TransactionType.AGGREGATE_BONDED',
				'aggregate.cosignature'
			]);

			// - aggregate
			const aggregateSchema = modelSchema[schemaName];
			expect(Object.keys(aggregateSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(aggregateSchema).to.contain.all.keys(['transactionsHash', 'transactions', 'cosignatures']);

			// - cosignature
			expect(modelSchema['aggregate.cosignature']).to.deep.equal({
				version: ModelType.uint64,
				signerPublicKey: ModelType.binary,
				signature: ModelType.binary,
				parentHash: ModelType.binary
			});
		};

		it('adds aggregateComplete system schema', () => assertAddsSchema('TransactionType.AGGREGATE_COMPLETE'));
		it('adds aggregateBonded system schema', () => assertAddsSchema('TransactionType.AGGREGATE_BONDED'));
	});
});
