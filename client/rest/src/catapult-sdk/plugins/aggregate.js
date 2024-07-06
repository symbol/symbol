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

/** @module plugins/aggregate */
import ModelType from '../model/ModelType.js';
import { models } from 'symbol-sdk/symbol';

/**
 * Creates an aggregate plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		const aggregateSchema = {
			transactionsHash: ModelType.binary,
			transactions: { type: ModelType.array, schemaName: 'transactionWithMetadata' },
			cosignatures: { type: ModelType.array, schemaName: 'aggregate.cosignature' }
		};

		builder.addTransactionSupport(models.TransactionType.AGGREGATE_COMPLETE, aggregateSchema);
		builder.addTransactionSupport(models.TransactionType.AGGREGATE_BONDED, aggregateSchema);

		builder.addSchema('aggregate.cosignature', {
			version: ModelType.uint64,
			signerPublicKey: ModelType.binary,
			signature: ModelType.binary,
			parentHash: ModelType.binary
		});
	}
};
