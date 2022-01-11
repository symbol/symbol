/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
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

const MultisigDb = require('../../../src/plugins/multisig/MultisigDb');
const dbTestUtils = require('../../db/utils/dbTestUtils');
const test = require('../../testUtils');
const MongoDb = require('mongodb');

const { Binary } = MongoDb;

const createMultisigEntry = (id, owner) => ({
	// simulated account is multisig with two cosigners and cosigns one multisig account
	_id: dbTestUtils.db.createObjectId(id),
	multisig: {
		accountAddress: new Binary(owner.address),
		cosignatoryAddresses: [new Binary(test.random.address()), new Binary(test.random.address())],
		multisigAddresses: [new Binary(test.random.address())]
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
