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

const AccountRestrictionsDb = require('../../../src/plugins/accountRestrictions/AccountRestrictionsDb');
const dbTestUtils = require('../../db/utils/dbTestUtils');
const test = require('../../testUtils');
const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');

const { Binary } = MongoDb;
const { EntityType } = catapult.model;

const createRestrictions = restrictions => {
	const restrictionsObject = [];

	let values = [];
	for (let i = 0; i < restrictions.numAddresses; ++i)
		values.push(new Binary(test.random.address()));

	restrictionsObject.push({
		restrictionType: 0.5 > Math.random() ? 1 : 129,
		values
	});

	values = [];
	for (let i = 0; i < restrictions.numMosaics; ++i)
		values.push(Math.floor(Math.random() * 1000));

	restrictionsObject.push({
		restrictionType: 0.5 > Math.random() ? 2 : 130,
		values
	});

	values = [];
	for (let i = 0; i < restrictions.numOperations; ++i) {
		const operationTypes = Object.keys(EntityType);
		values.push(EntityType[operationTypes[Math.floor(operationTypes.length * Math.random())]]);
	}

	restrictionsObject.push({
		restrictionType: 0.5 > Math.random() ? 4 : 132,
		values
	});

	return restrictionsObject;
};

const createAccountRestrictions = (address, restrictionsDescriptor) => {
	const accountRestrictions = {
		address: new Binary(address),
		restrictions: createRestrictions(restrictionsDescriptor)
	};
	return { _id: dbTestUtils.db.createObjectId(Math.floor(Math.random() * 10000)), meta: {}, accountRestrictions };
};

const accountRestrictionsDbTestUtils = {
	db: {
		createAccountRestrictions,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) => dbTestUtils.db.runDbTest(
			dbEntities,
			'accountRestrictions',
			db => new AccountRestrictionsDb(db),
			issueDbCommand,
			assertDbCommandResult
		)
	}
};
Object.assign(accountRestrictionsDbTestUtils, test);

module.exports = accountRestrictionsDbTestUtils;
