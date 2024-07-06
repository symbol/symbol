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

/** @module plugins/lockHash */
import ModelType from '../model/ModelType.js';
import { models } from 'symbol-sdk/symbol';

/**
 * Creates a lock hash plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addSchema('hashLockInfo', {
			id: ModelType.objectId,
			lock: { type: ModelType.object, schemaName: 'hashLockInfo.lock' }
		});
		builder.addSchema('hashLockInfo.lock', {
			version: ModelType.uint16,
			ownerAddress: ModelType.encodedAddress,
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64,
			endHeight: ModelType.uint64,
			status: ModelType.int,
			hash: ModelType.binary
		});

		builder.addTransactionSupport(models.TransactionType.HASH_LOCK, {
			mosaicId: ModelType.uint64HexIdentifier,
			amount: ModelType.uint64,
			duration: ModelType.uint64,
			hash: ModelType.binary
		});
	}
};
