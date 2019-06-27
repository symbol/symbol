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

const AccountPropertiesDb = require('../../../src/plugins/accountProperties/AccountPropertiesDb');
const catapult = require('catapult-sdk');
const dbTestUtils = require('../../db/utils/dbTestUtils');
const MongoDb = require('mongodb');
const test = require('../../testUtils');

const { Binary } = MongoDb;
const { EntityType } = catapult.model;

const createProperties = properties => {
	const propertiesObject = [];

	let values = [];
	for (let i = 0; i < properties.numAddresses; ++i)
		values.push(new Binary(test.random.address()));

	propertiesObject.push({
		propertyType: 0.5 > Math.random() ? 1 : 129,
		values
	});

	values = [];
	for (let i = 0; i < properties.numMosaics; ++i)
		values.push(Math.floor(Math.random() * 1000));

	propertiesObject.push({
		propertyType: 0.5 > Math.random() ? 2 : 130,
		values
	});

	values = [];
	for (let i = 0; i < properties.numEntityTypes; ++i) {
		const entityKeys = Object.keys(EntityType);
		values.push(EntityType[entityKeys[Math.floor(entityKeys.length * Math.random())]]);
	}

	propertiesObject.push({
		propertyType: 0.5 > Math.random() ? 4 : 132,
		values
	});

	return propertiesObject;
};

const createAccountProperties = (address, propertiesDescriptor) => {
	const accountProperties = {
		address: new Binary(address),
		properties: createProperties(propertiesDescriptor)
	};
	return { _id: dbTestUtils.db.createObjectId(Math.floor(Math.random() * 10000)), meta: {}, accountProperties };
};

const accountPropertiesDbTestUtils = {
	db: {
		createAccountProperties,
		runDbTest: (dbEntities, issueDbCommand, assertDbCommandResult) => dbTestUtils.db.runDbTest(
			dbEntities,
			'accountProperties',
			db => new AccountPropertiesDb(db),
			issueDbCommand,
			assertDbCommandResult
		)
	}
};
Object.assign(accountPropertiesDbTestUtils, test);

module.exports = accountPropertiesDbTestUtils;
