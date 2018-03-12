/**
 * Account id types.
 * @enum {string}
 * @exports db/AccountType
 */
const AccountType = {
	/** Account id is a public key */
	publicKey: 'publicKey',

	/** Account id is an address */
	address: 'address'
};

module.exports = AccountType;
