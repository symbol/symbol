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

class LockSecretDb {
	/**
	 * Creates LockSecretDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region lock retrieval

	/**
	 * Retrieves secret infos for given accounts filtered and paginated.
	 * @param {array<{Uint8Array}>} addresses Account addresses.
	 * @param {object} options Options for ordering and pagination. Can have an `offset`, and must contain the `sortField`, `sortDirection`,
	 * `pageSize` and `pageNumber`. 'sortField' must be within allowed 'sortingOptions'.
	 * @returns {Promise.<array>} Secret lock infos for all accounts.
	 */
	secretLocks(addresses, options) {
		const sortingOptions = { id: '_id' };
		const buffers = addresses.map(address => Buffer.from(address));
		const conditions = [{ 'lock.ownerAddress': { $in: buffers } }];

		if (options.offset)
			conditions.push({ [sortingOptions[options.sortField]]: { [1 === options.sortDirection ? '$gt' : '$lt']: options.offset } });

		const sortConditions = { $sort: { [sortingOptions[options.sortField]]: options.sortDirection } };
		return this.catapultDb.queryPagedDocuments_2(conditions, [], sortConditions, 'secretLocks', options);
	}

	/**
	 * Retrieves secret info for given secret.
	 * @param {Uint8Array} secret Secret hash.
	 * @returns {Promise.<object>} Secret lock info for a secret.
	 */
	secretLockBySecret(secret) {
		return this.catapultDb.queryDocument('secretLocks', { 'lock.secret': Buffer.from(secret) })
			.then(this.catapultDb.sanitizer.renameId);
	}

	// endregion
}

module.exports = LockSecretDb;
