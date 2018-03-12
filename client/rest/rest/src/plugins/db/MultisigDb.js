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
