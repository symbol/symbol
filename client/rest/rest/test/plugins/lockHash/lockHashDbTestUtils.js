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

const LockHashDb = require('../../../src/plugins/lockHash/LockHashDb');
const dbTestUtils = require('../../db/utils/dbTestUtils');
const test = require('../../testUtils');
const MongoDb = require('mongodb');

const { Binary } = MongoDb;

const createLockHash = (id, owner, hashPropertyName, value) => ({
	_id: dbTestUtils.db.createObjectId(id),
	meta: {},
	lock: {
		senderPublicKey: new Binary(owner.publicKey),
		senderAddress: new Binary(owner.address),
		[hashPropertyName]: new Binary(value)
	}
});

const createLockHashes = ((numRounds, owner, hashPropertyName, startdId = 0) => {
	const lockInfos = [];

	for (let i = 0; i < numRounds; ++i)
		lockInfos.push(createLockHash(startdId + i, owner, hashPropertyName, test.random.hash()));

	return lockInfos;
});

const lockHashDbTestUtils = {
	db: {
		createHashLock: (id, owner, value) => createLockHash(id, owner, 'hash', value),
		createHashLocks: (numRounds, owner, startdId = 0) => createLockHashes(numRounds, owner, 'hash', startdId),
		runDbTest: (lockName, dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, `${lockName}Locks`, db => new LockHashDb(db), issueDbCommand, assertDbCommandResult)
	}
};
Object.assign(lockHashDbTestUtils, test);

module.exports = lockHashDbTestUtils;
