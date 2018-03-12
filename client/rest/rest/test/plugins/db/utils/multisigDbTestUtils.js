const MongoDb = require('mongodb');
const MultisigDb = require('../../../../src/plugins/db/MultisigDb');
const test = require('../../../testUtils');
const dbTestUtils = require('../../../db/utils/dbTestUtils');

const { Binary } = MongoDb;

const createMultisigEntry = (id, owner) => ({
	// simulated account is multisig with two cosigners and cosigns one multisig account
	_id: dbTestUtils.db.createObjectId(id),
	multisig: {
		account: new Binary(owner.publicKey),
		accountAddress: new Binary(owner.address),
		cosignatories: [new Binary(test.random.publicKey()), new Binary(test.random.publicKey())],
		multisigAccounts: [new Binary(test.random.publicKey())]
	}
});

const multisigDbTestUtils = {
	db: {
		createMultisigEntry,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, 'multisigs', db => new MultisigDb(db), issueDbCommand, assertDbCommandResult)
	}
};
Object.assign(multisigDbTestUtils, test);

module.exports = multisigDbTestUtils;
