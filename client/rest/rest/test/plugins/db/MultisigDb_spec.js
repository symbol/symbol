import { expect } from 'chai';
import test from './utils/multisigDbTestUtils';

describe('multisig db', () => {
	describe('multisig by account', () => {
		it('returns undefined for account with no multisig entry', () => {
			// Arrange: create 4 + 1 random multisig entries
			const multisigEntries = test.db.createMultisigEntries(test.random.publicKey(), 4);

			// Assert:
			return test.db.runDbTest(
				multisigEntries,
				db => db.multisigByAccount(test.random.publicKey()),
				entity => { expect(entity).to.equal(undefined); });
		});

		it('returns multisig entry for account with multisig entry', () => {
			// Arrange: create 4 + 1 random multisig entries (last one has specified owner)
			const owner = test.random.publicKey();
			const multisigEntries = test.db.createMultisigEntries(owner, 4);

			// Assert:
			return test.db.runDbTest(
				multisigEntries,
				db => db.multisigByAccount(owner),
				entity => { expect(entity).to.deep.equal(multisigEntries[4]); });
		});
	});
});
