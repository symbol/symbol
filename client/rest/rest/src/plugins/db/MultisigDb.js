export default class MultisigDb {

	/**
	 * Creates MultisigDb around CatapultDb.
	 * @param {module:db/CatapultDb} db Catapult db instance.
	 */
	constructor(db) {
		this.catapultDb = db;
	}

	// region multisig retrieval

	/**
	 * Retrieves the multisig entry for a given account.
	 * @param {string} publicKey The account's public key.
	 * @returns {Promise.<object>} The account's multisig entry.
	 */
	multisigByAccount(publicKey) {
		const bufferPublicKey = Buffer.from(publicKey);
		const conditions = { account: bufferPublicKey };
		return this.catapultDb.queryDocument('multisigs', conditions)
			.then(this.catapultDb.sanitizer.deleteId);
	}

	// endregion
}
