import MongoDb from 'mongodb';
import MultisigDb from '../../../../src/plugins/db/MultisigDb';
import test from '../../../testUtils';
import dbTestUtils from '../../../db/utils/dbTestUtils';

const Binary = MongoDb.Binary;

function createMultisigEntry(id, owner) {
	// simulated account is multisig with two cosigners and cosigns one multisig account
	return {
		_id: dbTestUtils.db.createObjectId(id),
		account: new Binary(owner),
		cosignatories: [new Binary(test.random.publicKey()), new Binary(test.random.publicKey())],
		multisigAccounts: [new Binary(test.random.publicKey())]
	};
}

function createMultisigEntries(knownOwner, numRandomEntries) {
	const multisigEntries = [];

	// add random entries (with even ids)
	const randomOwner = test.random.publicKey();
	for (let dbId = 0; dbId < numRandomEntries; ++dbId) {
		randomOwner[0] = dbId;
		multisigEntries.push(createMultisigEntry(2 * dbId, randomOwner));
	}

	// add entry for known owner (with odd id)
	multisigEntries.push(createMultisigEntry(3, knownOwner));

	return multisigEntries;
}

const multisigDbTestUtils = {
	db: {
		createMultisigEntry,
		createMultisigEntries,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, 'multisigs', db => new MultisigDb(db), issueDbCommand, assertDbCommandResult)
	}
};
Object.assign(multisigDbTestUtils, test);

export default multisigDbTestUtils;
