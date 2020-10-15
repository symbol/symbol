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

const { buildOffsetCondition } = require('../../db/dbUtils');

class LockHashDb {
	/**
	 * Creates LockHashDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region lock retrieval

	/**
	 * Retrieves hash lock infos for given accounts filtered and paginated.
	 * @param {array<{Uint8Array}>} addresses Account addresses.
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<array>} Hash lock infos for all accounts.
	 */
	hashLocks(addresses, options) {
		const sortingOptions = { id: '_id' };
		const buffers = addresses.map(address => Buffer.from(address));
		let conditions = { 'lock.ownerAddress': { $in: buffers } };

		const offsetCondition = buildOffsetCondition(options, sortingOptions);
		if (offsetCondition)
			conditions = Object.assign(conditions, offsetCondition);

		const sortConditions = { [sortingOptions[options.sortField]]: options.sortDirection };
		return this.catapultDb.queryPagedDocuments(conditions, [], sortConditions, 'hashLocks', options);
	}

	/**
	 * Retrieves hash info for given hash.
	 * @param {Uint8Array} hash Lock hash.
	 * @returns {Promise.<object>} Hash lock info for a hash.
	 */
	hashLockByHash(hash) {
		return this.catapultDb.queryDocument('hashLocks', { 'lock.hash': Buffer.from(hash) })
			.then(this.catapultDb.sanitizer.renameId);
	}

	// endregion
}

module.exports = LockHashDb;
