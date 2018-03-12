const MongoDb = require('mongodb');
const LockDb = require('../../../../src/plugins/db/LockDb');
const test = require('../../../testUtils');
const dbTestUtils = require('../../../db/utils/dbTestUtils');

const { Binary } = MongoDb;

const createLockInfo = (id, owner) => ({
	_id: dbTestUtils.db.createObjectId(id),
	lock: {
		account: new Binary(owner.publicKey),
		accountAddress: new Binary(owner.address)
	}
});

const lockDbTestUtils = {
	db: {
		createLockInfo,
		runDbTest: (lockName, dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, `${lockName}LockInfos`, db => new LockDb(db), issueDbCommand, assertDbCommandResult)
	}
};
Object.assign(lockDbTestUtils, test);

module.exports = lockDbTestUtils;
