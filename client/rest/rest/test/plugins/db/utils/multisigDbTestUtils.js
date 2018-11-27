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
const MongoDb = require('mongodb');
const MultisigDb = require('../../../../src/plugins/db/MultisigDb');
const test = require('../../../testUtils');

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
