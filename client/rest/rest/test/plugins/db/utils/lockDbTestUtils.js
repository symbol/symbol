/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

const dbTestUtils = require('../../../db/utils/dbTestUtils');
const LockDb = require('../../../../src/plugins/db/LockDb');
const MongoDb = require('mongodb');
const test = require('../../../testUtils');

const { Binary } = MongoDb;

const createLockInfo = (id, owner, hashPropertyName, value) => ({
	_id: dbTestUtils.db.createObjectId(id),
	meta: {},
	lock: {
		account: new Binary(owner.publicKey),
		accountAddress: new Binary(owner.address),
		[hashPropertyName]: new Binary(value)
	}
});

const createLockInfos = ((numRounds, owner, hashPropertyName, startdId = 0) => {
	const lockInfos = [];

	for (let i = 0; i < numRounds; ++i)
		lockInfos.push(createLockInfo(startdId + i, owner, hashPropertyName, test.random.hash()));

	return lockInfos;
});

const lockDbTestUtils = {
	db: {
		createHashLockInfo: (id, owner, value) => createLockInfo(id, owner, 'hash', value),
		createHashLockInfos: (numRounds, owner, startdId = 0) => createLockInfos(numRounds, owner, 'hash', startdId),
		createSecretLockInfo: (id, owner, value) => createLockInfo(id, owner, 'secret', value),
		createSecretLockInfos: (numRounds, owner, startdId = 0) => createLockInfos(numRounds, owner, 'secret', startdId),
		runDbTest: (lockName, dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, `${lockName}LockInfos`, db => new LockDb(db), issueDbCommand, assertDbCommandResult)
	}
};
Object.assign(lockDbTestUtils, test);

module.exports = lockDbTestUtils;
