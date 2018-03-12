const { expect } = require('chai');
const test = require('../../../testUtils');

const createOwner = test.random.account;

const createEntries = (createEntry, knownOwner, numRandomEntries) => {
	const entries = [];

	// add random entries (with even ids)
	const randomAccount = createOwner();
	for (let dbId = 0; dbId < numRandomEntries; ++dbId) {
		randomAccount[0] = dbId;
		entries.push(createEntry(2 * dbId, randomAccount));
	}

	// add entry for known owner (with odd id)
	entries.push(createEntry(3, knownOwner));

	return entries;
};

const addTests = traits => {
	const runEntriesByAccountsTest = (additionalOwners, accountsToQuery, expectedIndexes) => {
		// Arrange:
		const entries = createEntries(traits.createEntry, createOwner(), 4);
		let i = 0;
		additionalOwners.forEach(account => {
			entries.push(traits.createEntry(10 + i++, account));
		});

		// Assert:
		return traits.runDbTest(
			entries,
			accountsToQuery,
			entities => expect(entities).to.deep.equal(expectedIndexes.map(index => entries[index]))
		);
	};

	it('returns empty array if there are no entries', () =>
		// Assert:
		runEntriesByAccountsTest([], [createOwner(), createOwner(), createOwner()].map(owner => traits.toDbApiId(owner)), []));

	it('returns single matching entry', () => {
		// Arrange:
		const account = createOwner();

		// Assert: first five entries are the seeds
		return runEntriesByAccountsTest([account], [traits.toDbApiId(account)], [5]);
	});

	it('returns multiple matching entries (of a single account)', () => {
		// Arrange:
		const account = createOwner();

		// Assert: first five entries are the seeds
		const accountId = traits.toDbApiId(account);
		return runEntriesByAccountsTest([account, account, account], [accountId], [5, 6, 7]);
	});

	it('returns multiple matching entries (one per account)', () => {
		// Arrange:
		const accounts = [];
		for (let i = 0; 6 > i; ++i)
			accounts.push(createOwner());

		// Assert: first five entries are the seeds
		const queriedIds = [0, 3, 5].map(index => traits.toDbApiId(accounts[index]));
		return runEntriesByAccountsTest(accounts, queriedIds, [5, 8, 10]);
	});

	it('returns only matching entries', () => {
		// Arrange:
		const accounts = [createOwner(), createOwner()];

		// Assert: first five entries are the seeds
		const queriedIds = [accounts[0], createOwner(), accounts[1], createOwner()].map(owner => traits.toDbApiId(owner));
		return runEntriesByAccountsTest(accounts, queriedIds, [5, 6]);
	});
};

module.exports = { addTests };
