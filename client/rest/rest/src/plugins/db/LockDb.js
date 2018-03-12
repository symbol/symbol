const AccountType = require('../AccountType');

class LockDb {
	/**
	 * Creates LockDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region lock retrieval

	/**
	 * Retrieves hash infos for given accounts.
	 * @param {module:db/AccountType} type The type of account ids.
	 * @param {array<object>} accountIds The account ids.
	 * @returns {Promise.<array>} The hash lock infos for all accounts.
	 */
	hashLocksByAccounts(type, accountIds) {
		return this.locksByAccounts('hash', type, accountIds);
	}

	/**
	 * Retrieves secret infos for given accounts.
	 * @param {module:db/AccountType} type The type of account ids.
	 * @param {array<object>} accountIds The account ids.
	 * @returns {Promise.<array>} The secret lock infos for all accounts.
	 */
	secretLocksByAccounts(type, accountIds) {
		return this.locksByAccounts('secret', type, accountIds);
	}

	/**
	 * @param {string} lockName The type of lock.
	 * @param {module:db/AccountType} type The type of account ids.
	 * @param {array<object>} accountIds The account ids.
	 * @returns {Promise.<array>} The lock infos for all accounts.
	 */
	locksByAccounts(lockName, type, accountIds) {
		const buffers = accountIds.map(accountId => Buffer.from(accountId));
		const fieldName = (AccountType.publicKey === type) ? 'lock.account' : 'lock.accountAddress';
		return this.catapultDb.queryDocuments(`${lockName}LockInfos`, { [fieldName]: { $in: buffers } });
	}

	// endregion
}

module.exports = LockDb;
