const AccountType = require('../../../src/plugins/AccountType');
const entitiesByAccounts = require('./utils/entriesByAccountsTestUtils');
const test = require('./utils/lockDbTestUtils');

describe('lock db', () => {
	const addLockTests = (name, lockExtractor) => {
		const createLockTraits = (toDbApiId, typedLockExtractor) => ({
			createEntry: (id, account) => test.db.createLockInfo(id, account),
			toDbApiId,
			runDbTest: (entries, accountsToQuery, assertDbCommandResult) => test.db.runDbTest(
				name,
				entries,
				db => typedLockExtractor(db, accountsToQuery),
				assertDbCommandResult
			)
		});

		describe('by publicKey', () => {
			entitiesByAccounts.addTests(createLockTraits(
				owner => owner.publicKey,
				(db, accountsToQuery) => lockExtractor(db, AccountType.publicKey, accountsToQuery)
			));
		});

		describe('by address', () => {
			entitiesByAccounts.addTests(createLockTraits(
				owner => owner.address,
				(db, accountsToQuery) => lockExtractor(db, AccountType.address, accountsToQuery)
			));
		});
	};

	describe('hash locks', () => {
		addLockTests(
			'hash',
			(db, type, accountsToQuery) => db.hashLocksByAccounts(type, accountsToQuery)
		);
	});

	describe('secret locks', () => {
		addLockTests(
			'secret',
			(db, type, accountsToQuery) => db.secretLocksByAccounts(type, accountsToQuery)
		);
	});
});
