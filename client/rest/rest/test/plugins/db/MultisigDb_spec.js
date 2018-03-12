const AccountType = require('../../../src/plugins/AccountType');
const entitiesByAccounts = require('./utils/entriesByAccountsTestUtils');
const test = require('./utils/multisigDbTestUtils');

describe('multisig db', () => {
	describe('multisigs by publicKey', () =>
		entitiesByAccounts.addTests({
			createEntry: (id, account) => test.db.createMultisigEntry(id, account),
			toDbApiId: owner => owner.publicKey,
			runDbTest: (entries, accountsToQuery, assertDbCommandResult) => test.db.runDbTest(
				entries,
				db => db.multisigsByAccounts(AccountType.publicKey, accountsToQuery),
				assertDbCommandResult
			)
		}));

	describe('multisigs by address', () =>
		entitiesByAccounts.addTests({
			createEntry: (id, account) => test.db.createMultisigEntry(id, account),
			toDbApiId: owner => owner.address,
			runDbTest: (entries, accountsToQuery, assertDbCommandResult) => test.db.runDbTest(
				entries,
				db => db.multisigsByAccounts(AccountType.address, accountsToQuery),
				assertDbCommandResult
			)
		}));
});
