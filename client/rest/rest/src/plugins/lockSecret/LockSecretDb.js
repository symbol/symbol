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

const AccountType = require('../AccountType');

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
	 * Retrieves secret infos for given accounts.
	 * @param {module:db/AccountType} type Type of account ids.
	 * @param {array<object>} accountIds Account ids.
	 * @param {string} id Paging id.
	 * @param {int} pageSize Page size.
	 * @param {object} options Additional options.
	 * @returns {Promise.<array>} Secret lock infos for all accounts.
	 */
	secretLocksByAccounts(type, accountIds, id, pageSize, options) {
		const buffers = accountIds.map(accountId => Buffer.from(accountId));
		const fieldName = (AccountType.publicKey === type) ? 'lock.senderPublicKey' : 'lock.senderAddress';
		const conditions = { $and: [{ [fieldName]: { $in: buffers } }] };
		return this.catapultDb.queryPagedDocuments('secretLockInfos', conditions, id, pageSize, options)
			.then(this.catapultDb.sanitizer.copyAndDeleteIds);
	}

	/**
	 * Retrieves secret info for given secret.
	 * @param {Uint8Array} secret Secret hash.
	 * @returns {Promise.<object>} Secret lock info for a secret.
	 */
	secretLockBySecret(secret) {
		return this.catapultDb.queryDocument('secretLockInfos', { 'lock.secret': Buffer.from(secret) })
			.then(this.catapultDb.sanitizer.copyAndDeleteId);
	}

	// endregion
}

module.exports = LockSecretDb;
