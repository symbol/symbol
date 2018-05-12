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

class MultisigDb {
	/**
	 * Creates MultisigDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region multisig retrieval

	/**
	 * Retrieves the multisig entries for given accounts.
	 * @param {module:db/AccountType} type The type of account ids.
	 * @param {array<object>} accountIds The account ids.
	 * @returns {Promise.<array>} The multisig entries for all accounts.
	 */
	multisigsByAccounts(type, accountIds) {
		const buffers = accountIds.map(accountId => Buffer.from(accountId));
		const fieldName = (AccountType.publicKey === type) ? 'multisig.account' : 'multisig.accountAddress';
		return this.catapultDb.queryDocuments('multisigs', { [fieldName]: { $in: buffers } });
	}

	// endregion
}

module.exports = MultisigDb;
