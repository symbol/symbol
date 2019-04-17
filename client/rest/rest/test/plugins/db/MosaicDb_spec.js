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

const catapult = require('catapult-sdk');
const CatapultDb = require('../../../src/db/CatapultDb');
const dbUtils = require('../../../src/db/dbUtils');
const MosaicDb = require('../../../src/plugins/db/MosaicDb');
const test = require('./utils/mosaicDbTestUtils');
const testDbOptions = require('../../db/utils/testDbOptions');
const { expect } = require('chai');

const { convertToLong } = dbUtils;

describe('mosaic db', () => {
	const createMosaics = (numNamespaces, numMosaicsPerNamespace) => {
		const owner = test.random.publicKey();
		return test.db.createMosaics(owner, numNamespaces, numMosaicsPerNamespace);
	};

	const sanitizeDbEntities = entities => entities.forEach(item => delete item._id);

	const populateCollection = (db, collectionName, entities) =>
		db.database.collection(collectionName)
			.drop()
			.catch(() => Promise.resolve())
			.then(() => db.database.collection(collectionName)[Array.isArray(entities) ? 'insertMany' : 'insertOne'](entities));

	describe('mosaics by ids', () => {
		it('returns empty array for unknown mosaic ids', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[123, 456]]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns single matching mosaic', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0]]),
				entities => { expect(entities).to.deep.equal([mosaics[10]]); }
			);
		});

		it('returns multiple matching mosaics', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0], [10007, 0], [10003, 0]]),
				entities => { expect(entities).to.deep.equal([mosaics[10], mosaics[7], mosaics[3]]); }
			);
		});

		it('returns only known mosaics', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0], [10021, 0], [10003, 0]]),
				entities => expect(entities).to.deep.equal([mosaics[10], mosaics[3]])
			);
		});
	});

	describe('namespaces by mosaics ids', () => {
		it('returns namespaces by mosaic ids', () => {
			// Arrange:
			const aliasType = catapult.model.namespace.aliasType.mosaic;
			const namespace1 = test.namespacesDb.createNamespace(12345, 76756, aliasType, 1, { start: 0, end: 100 });
			const namespace2 = test.namespacesDb.createNamespace(67891, 87823, aliasType, 2, { start: 0, end: 100 });
			const namespace3 = test.namespacesDb.createNamespace(34567, 57231, aliasType, 3, { start: 0, end: 100 });

			// Act + Assert:
			return test.namespacesDb.runDbTest(
				[namespace1, namespace2, namespace3],
				db => db.activeNamespacesByMosaicsIds([76756, 87823]),
				entities => { expect(entities).to.deep.equal([namespace1, namespace2]); }
			);
		});

		it('returns namespaces only with mosaic alias type', () => {
			// Arrange:
			const aliasType = catapult.model.namespace.aliasType.mosaic;
			const namespace1 = test.namespacesDb.createNamespace(89654, 11111, aliasType, 1, { start: 0, end: 100 });
			const namespace2 = test.namespacesDb.createNamespace(34452, 11111, aliasType + 5, 1, { start: 0, end: 100 });

			// Act + Assert:
			return test.namespacesDb.runDbTest(
				[namespace1, namespace2],
				db => db.activeNamespacesByMosaicsIds([11111]),
				entities => { expect(entities).to.deep.equal([namespace1]); }
			);
		});

		it('returns empty for no namespaces with mosaic id', () => {
			// Arrange:
			const aliasType = catapult.model.namespace.aliasType.mosaic;
			const namespace1 = test.namespacesDb.createNamespace(76598, 34023, aliasType, 1, { start: 0, end: 100 });
			const namespace2 = test.namespacesDb.createNamespace(78520, 54602, aliasType, 2, { start: 0, end: 100 });

			// Act + Assert:
			return test.namespacesDb.runDbTest(
				[namespace1, namespace2],
				db => db.activeNamespacesByMosaicsIds([12121]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('does not return expired namespaces', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new MosaicDb(db);
			const chainHeight = 5;
			const mosaicId = 11111;
			const aliasType = catapult.model.namespace.aliasType.mosaic;
			const namespace1 = test.namespacesDb.createNamespace(89034, mosaicId, aliasType, 1, { start: 0, end: 6 });
			const namespace2 = test.namespacesDb.createNamespace(10784, mosaicId, aliasType, 1, { start: 0, end: -1 });
			const namespace3 = test.namespacesDb.createNamespace(30753, mosaicId, aliasType, 1, { start: 0, end: 5 });
			const namespace4 = test.namespacesDb.createNamespace(30753, mosaicId, aliasType, 1, { start: 0, end: 4 });

			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(db, 'blocks', [...Array(chainHeight)].map(() => ({}))))
				.then(() => populateCollection(db, 'namespaces', [namespace1, namespace2, namespace3, namespace4]))
				.then(() => sanitizeDbEntities([namespace1, namespace2, namespace3, namespace4]))
				.then(() => dbFacade.activeNamespacesByMosaicsIds([mosaicId]))
				.then(entities => { expect(entities).to.deep.equal([namespace1, namespace2]); })
				.then(() => db.close());
		});
	});

	describe('register namespace transactions by namespace ids', () => {
		// Arrange:
		const registerNamespaceTransaction1 = test.registerNamespaceTransactionDb
			.createRegisterNamespaceTransaction(12345, catapult.model.EntityType.registerNamespace, 'alias_1');
		const registerNamespaceTransaction2 = test.registerNamespaceTransactionDb
			.createRegisterNamespaceTransaction(67891, catapult.model.EntityType.registerNamespace, 'alias_2');
		const registerNamespaceTransaction3 = test.registerNamespaceTransactionDb
			.createRegisterNamespaceTransaction(67891, 123, 'alias_2');

		it('returns register namespace transactions by namespace ids', () =>
			test.registerNamespaceTransactionDb.runDbTest(
				[registerNamespaceTransaction1, registerNamespaceTransaction2, registerNamespaceTransaction3],
				db => db.registerNamespaceTransactionsByNamespaceIds([convertToLong(12345), convertToLong(67891)]),
				entities => { expect(entities).to.deep.equal([registerNamespaceTransaction1, registerNamespaceTransaction2]); }
			));

		it('returns empty for no register namespace transactions with namespaceId', () =>
			test.registerNamespaceTransactionDb.runDbTest(
				[registerNamespaceTransaction1, registerNamespaceTransaction2, registerNamespaceTransaction3],
				db => db.registerNamespaceTransactionsByNamespaceIds([convertToLong(34343)]),
				entities => { expect(entities).to.deep.equal([]); }
			));
	});
});
