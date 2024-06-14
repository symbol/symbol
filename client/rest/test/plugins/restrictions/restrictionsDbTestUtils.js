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

import RestrictionsDb from '../../../src/plugins/restrictions/RestrictionsDb.js';
import dbTestUtils from '../../db/utils/dbTestUtils.js';
import test from '../../testUtils.js';
import MongoDb from 'mongodb';
import { models } from 'symbol-sdk/symbol';

const { Binary } = MongoDb;

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
		const operationTypes = Object.keys(models.TransactionType);
		values.push(models.TransactionType[operationTypes[Math.floor(operationTypes.length * Math.random())]]);
	}

	restrictionsObject.push({
		restrictionType: 0.5 > Math.random() ? 4 : 132,
		values
	});

	return restrictionsObject;
};

const restrictionsDbTestUtils = {
	accountDb: {
		createAccountRestrictions: (address, restrictionsDescriptor) => {
			const accountRestrictions = {
				address: new Binary(address),
				restrictions: createRestrictions(restrictionsDescriptor)
			};
			return { _id: dbTestUtils.db.createObjectId(Math.floor(Math.random() * 10000)), meta: {}, accountRestrictions };
		},

		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) => dbTestUtils.db.runDbTest(
			dbEntities,
			'accountRestrictions',
			db => new RestrictionsDb(db),
			issueDbCommand,
			assertDbCommandResult
		)
	}
};

Object.assign(restrictionsDbTestUtils, test);

export default restrictionsDbTestUtils;
