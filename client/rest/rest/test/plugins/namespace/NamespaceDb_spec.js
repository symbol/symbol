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

const test = require('./namespaceDbTestUtils');
const CatapultDb = require('../../../src/db/CatapultDb');
const dbUtils = require('../../../src/db/dbUtils');
const NamespaceDb = require('../../../src/plugins/namespace/NamespaceDb');
const dbTestUtils = require('../../db/utils/dbTestUtils');
const testDbOptions = require('../../db/utils/testDbOptions');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { address } = catapult.model;
const { uint64 } = catapult.utils;
const { convertToLong } = dbUtils;
const { Binary } = MongoDb;

describe('namespace db', () => {
	describe('namespaces', () => {
		const { createObjectId } = dbTestUtils.db;

		const runNamespacesDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, 'namespaces', db => new NamespaceDb(db), issueDbCommand, assertDbCommandResult);

		const level0Test1 = uint64.fromHex('85BBEA6CC462B244');
		const level0Test2 = uint64.fromHex('3C2437767AF232DC');
		const ownerAddressTest1 = address.stringToAddress('SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ');
		const ownerAddressTest2 = address.stringToAddress('NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA');

		const paginationOptions = {
			pageSize: 10,
			pageNumber: 1,
			sortField: 'id',
			sortDirection: -1
		};

		const createNamespace = (objectId, aliasType, level0, ownerAddress, registrationType, active = true) => ({
			_id: createObjectId(objectId),
			meta: { active },
			namespace: {
				alias: { type: aliasType },
				level0: convertToLong(level0),
				ownerAddress: ownerAddress ? Buffer.from(ownerAddress) : undefined,
				registrationType
			}
		});

		const runTestAndVerifyIds = (dbNamespaces, dbQuery, expectedIds) => {
			const expectedObjectIds = expectedIds.map(id => createObjectId(id));

			return runNamespacesDbTest(
				dbNamespaces,
				dbQuery,
				namespacesPage => {
					const returnedIds = namespacesPage.data.map(t => t.id);
					expect(namespacesPage.data.length).to.equal(expectedObjectIds.length);
					expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
				}
			);
		};

		it('returns expected structure', () => {
			// Arrange:
			const dbNamespaces = [createNamespace(10, 1, level0Test1, ownerAddressTest1, 0)];

			// Act + Assert:
			return runNamespacesDbTest(
				dbNamespaces,
				db => db.namespaces(undefined, undefined, undefined, undefined, paginationOptions),
				page => {
					const expected_keys = ['id', 'meta', 'namespace'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
				}
			);
		});

		describe('return empty array for unknown param', () => {
			// Arrange:
			const dbNamespaces = () => [createNamespace(10, 1, level0Test1, ownerAddressTest1, 0)];

			it('aliasType', () =>
				runTestAndVerifyIds(dbNamespaces(), db => db.namespaces(2, undefined, undefined, undefined, paginationOptions), []));

			it('level0', () =>
				runTestAndVerifyIds(
					dbNamespaces(),
					db => db.namespaces(undefined, level0Test2, undefined, undefined, paginationOptions), []
				));

			it('ownerAddress', () =>
				runTestAndVerifyIds(
					dbNamespaces(),
					db => db.namespaces(undefined, undefined, ownerAddressTest2, undefined, paginationOptions), []
				));

			it('registrationType', () =>
				runTestAndVerifyIds(dbNamespaces(), db => db.namespaces(undefined, undefined, undefined, 1, paginationOptions), []));
		});

		describe('returns filtered namespaces by param', () => {
			// Arrange:
			const dbNamespaces = () => [
				createNamespace(10, 2, level0Test1, ownerAddressTest1, 0),
				createNamespace(20, 1, level0Test2, ownerAddressTest1, 0),
				createNamespace(30, 1, level0Test1, ownerAddressTest2, 0),
				createNamespace(40, 1, level0Test1, ownerAddressTest1, 1)
			];

			it('aliasType', () =>
				runTestAndVerifyIds(
					dbNamespaces(),
					db => db.namespaces(2, undefined, undefined, undefined, paginationOptions), [10]
				));

			it('level0', () =>
				runTestAndVerifyIds(
					dbNamespaces(),
					db => db.namespaces(undefined, level0Test2, undefined, undefined, paginationOptions), [20]
				));

			it('ownerAddress', () =>
				runTestAndVerifyIds(
					dbNamespaces(),
					db => db.namespaces(undefined, undefined, ownerAddressTest2, undefined, paginationOptions), [30]
				));

			it('registrationType', () =>
				runTestAndVerifyIds(
					dbNamespaces(),
					db => db.namespaces(undefined, undefined, undefined, 1, paginationOptions), [40]
				));
		});

		it('returns all namepsaces if no params are provided', () => {
			// Arrange:
			const dbNamespaces = [
				createNamespace(10, 2, level0Test1, ownerAddressTest1, 0),
				createNamespace(20, 1, level0Test2, ownerAddressTest2, 0),
				createNamespace(30, 1, level0Test2, ownerAddressTest1, 1)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbNamespaces,
				db => db.namespaces(undefined, undefined, undefined, undefined, paginationOptions),
				[10, 20, 30]
			);
		});

		it('only returns active namespaces', () => {
			// Arrange:
			const dbNamespaces = [
				createNamespace(10, 1, level0Test1, ownerAddressTest1, 0, false),
				createNamespace(20, 1, level0Test2, ownerAddressTest1, 0, true)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbNamespaces,
				db => db.namespaces(undefined, undefined, undefined, undefined, paginationOptions),
				[20]
			);
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbNamespaces = () => [
				createNamespace(10, 1, level0Test1, ownerAddressTest1, 0),
				createNamespace(20, 2, level0Test1, ownerAddressTest1, 0),
				createNamespace(30, 0, level0Test1, ownerAddressTest1, 0)
			];

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runNamespacesDbTest(
					dbNamespaces(),
					db => db.namespaces(undefined, undefined, undefined, undefined, options),
					page => {
						expect(page.data[0].id).to.deep.equal(createObjectId(10));
						expect(page.data[1].id).to.deep.equal(createObjectId(20));
						expect(page.data[2].id).to.deep.equal(createObjectId(30));
					}
				);
			});

			it('direction descending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: -1
				};

				// Act + Assert:
				return runNamespacesDbTest(
					dbNamespaces(),
					db => db.namespaces(undefined, undefined, undefined, undefined, options),
					page => {
						expect(page.data[0].id).to.deep.equal(createObjectId(30));
						expect(page.data[1].id).to.deep.equal(createObjectId(20));
						expect(page.data[2].id).to.deep.equal(createObjectId(10));
					}
				);
			});

			it('sort field', () => {
				const queryPagedDocumentsSpy = sinon.spy(CatapultDb.prototype, 'queryPagedDocuments');
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runNamespacesDbTest(
					dbNamespaces(),
					db => db.namespaces(undefined, undefined, undefined, undefined, options),
					() => {
						expect(queryPagedDocumentsSpy.calledOnce).to.equal(true);
						expect(Object.keys(queryPagedDocumentsSpy.firstCall.args[2])[0]).to.equal('_id');
						queryPagedDocumentsSpy.restore();
					}
				);
			});
		});

		describe('respects offset', () => {
			// Arrange:
			const dbNamespaces = () => [
				createNamespace(10, 1, level0Test1, ownerAddressTest1, 0),
				createNamespace(20, 2, level0Test1, ownerAddressTest1, 0),
				createNamespace(30, 0, level0Test1, ownerAddressTest1, 0)
			];
			const options = {
				pageSize: 10,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1,
				offset: createObjectId(20)
			};

			it('gt', () => {
				options.sortDirection = 1;

				// Act + Assert:
				return runTestAndVerifyIds(dbNamespaces(), db => db.namespaces(undefined, undefined, undefined, undefined, options), [30]);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(dbNamespaces(), db => db.namespaces(undefined, undefined, undefined, undefined, options), [10]);
			});
		});
	});

	const createOwner = test.random.account;

	describe('namespace by id', () => {
		const { createObjectId } = dbTestUtils.db;

		it('returns undefined for unknown namespace id', () => {
			// Arrange: create 5 namespaces and 5 inactive namespaces
			const owner = createOwner();
			const namespaces = test.db.createNamespaces(5, owner);

			// Assert:
			return test.db.runDbTest(
				namespaces,
				db => db.namespaceById([123, 456]),
				entity => { expect(entity).to.equal(undefined); }
			);
		});

		it('returns root namespace for known level 0 namespace id', () => {
			// Arrange: create 5 namespaces and 5 inactive namespaces
			const owner = createOwner();
			const namespaces = test.db.createNamespaces(5, owner);

			// Assert:
			return test.db.runDbTest(
				namespaces,
				db => db.namespaceById([12303, 0]),
				entity => {
					expect(entity.id).to.deep.equal(createObjectId(3));
					expect(entity.meta.active).to.equal(true);
					expect(entity.namespace).to.deep.equal(namespaces[3].namespace);
				}
			);
		});

		it('returns level 1 child namespace for known level 1 namespace id', () => {
			// Arrange: create 5 namespaces and 5 inactive namespaces
			const owner = createOwner();
			const namespaces = test.db.createNamespaces(5, owner);

			// Assert:
			return test.db.runDbTest(
				namespaces,
				db => db.namespaceById([12301, 0]),
				entity => {
					expect(entity.id).to.deep.equal(createObjectId(1));
					expect(entity.meta.active).to.equal(true);
					expect(entity.namespace).to.deep.equal(namespaces[1].namespace);
				}
			);
		});

		it('returns level 2 child  namespace for known level 2 namespace id', () => {
			// Arrange: create 5 namespaces and 5 inactive namespaces
			const owner = createOwner();
			const namespaces = test.db.createNamespaces(5, owner);

			// Assert:
			return test.db.runDbTest(
				namespaces,
				db => db.namespaceById([12302, 0]),
				entity => {
					expect(entity.id).to.deep.equal(createObjectId(2));
					expect(entity.meta.active).to.equal(true);
					expect(entity.namespace).to.deep.equal(namespaces[2].namespace);
				}
			);
		});
	});

	const sanitizeDbEntities = entities => entities.forEach(item => delete item._id);

	const populateCollection = (db, collectionName, entities) =>
		db.database.collection(collectionName)
			.drop()
			.catch(() => Promise.resolve())
			.then(() => db.database.collection(collectionName)[Array.isArray(entities) ? 'insertMany' : 'insertOne'](entities));

	describe('activeNamespacesWithAlias', () => {
		const aliasTypeMosaic = catapult.model.namespace.aliasType.mosaic;
		const aliasTypeAddress = catapult.model.namespace.aliasType.address;
		const testAddress = {
			one: 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ',
			two: 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA',
			three: 'SAAAIBC7AM65HOFDLYGFUT46H44TROZ7MUWCW6I'
		};
		const lifetime = { start: 0, end: 100 };
		const createNamespace = (namespaceId, aliasTarget, aliasType, depth, expirationHeight) => ({
			namespace: {
				depth,
				level0: 1 >= depth ? convertToLong(namespaceId) : '',
				level1: 2 === depth ? convertToLong(namespaceId) : '',
				level2: 3 === depth ? convertToLong(namespaceId) : '',
				alias: {
					type: aliasType,
					mosaicId: aliasType === catapult.model.namespace.aliasType.mosaic ? convertToLong(aliasTarget) : null,
					address: aliasType === catapult.model.namespace.aliasType.address
						? new Binary(Buffer.from(address.stringToAddress(aliasTarget)))
						: null
				},
				startHeight: convertToLong(expirationHeight.start),
				endHeight: convertToLong(expirationHeight.end)
			}
		});

		it('returns namespaces by mosaic ids', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new NamespaceDb(db);
			const namespace1 = createNamespace(12345, 76756, aliasTypeMosaic, 1, lifetime);
			const namespace2 = createNamespace(67891, 87823, aliasTypeMosaic, 2, lifetime);
			const namespace3 = createNamespace(34567, 57231, aliasTypeMosaic, 3, lifetime);
			const namespace4 = createNamespace(67556, testAddress.one, aliasTypeAddress, 1, lifetime);

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(db, 'namespaces', [namespace1, namespace2, namespace3, namespace4]))
				.then(() => sanitizeDbEntities([namespace1, namespace2, namespace3, namespace4]))
				.then(() => dbFacade.activeNamespacesWithAlias(aliasTypeMosaic, [76756, 87823]))
				.then(entities => { expect(entities).to.deep.equal([namespace1, namespace2]); })
				.then(() => db.close());
		});

		it('returns namespaces by addresses', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new NamespaceDb(db);
			const namespace1 = createNamespace(12345, testAddress.one, aliasTypeAddress, 1, lifetime);
			const namespace2 = createNamespace(67891, testAddress.two, aliasTypeAddress, 2, lifetime);
			const namespace3 = createNamespace(34567, testAddress.three, aliasTypeAddress, 3, lifetime);
			const namespace4 = createNamespace(12345, 76756, aliasTypeMosaic, 1, lifetime);

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(db, 'namespaces', [namespace1, namespace2, namespace3, namespace4]))
				.then(() => sanitizeDbEntities([namespace1, namespace2, namespace3, namespace4]))
				.then(() => dbFacade.activeNamespacesWithAlias(
					aliasTypeAddress,
					[
						address.stringToAddress(testAddress.one),
						address.stringToAddress(testAddress.two)
					]
				))
				.then(entities => { expect(entities).to.deep.equal([namespace1, namespace2]); })
				.then(() => db.close());
		});

		it('returns empty for no namespaces with mosaic id', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new NamespaceDb(db);
			const namespace1 = createNamespace(12345, 76756, aliasTypeMosaic, 1, lifetime);
			const namespace2 = createNamespace(67891, 87823, aliasTypeMosaic, 2, lifetime);

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(db, 'namespaces', [namespace1, namespace2]))
				.then(() => sanitizeDbEntities([namespace1, namespace2]))
				.then(() => dbFacade.activeNamespacesWithAlias(aliasTypeMosaic, [12121]))
				.then(entities => { expect(entities).to.deep.equal([]); })
				.then(() => db.close());
		});

		it('returns empty for no namespaces with address id', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new NamespaceDb(db);
			const namespace1 = createNamespace(12345, testAddress.one, aliasTypeAddress, 1, lifetime);
			const namespace2 = createNamespace(67891, testAddress.two, aliasTypeAddress, 2, lifetime);

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(db, 'namespaces', [namespace1, namespace2]))
				.then(() => sanitizeDbEntities([namespace1, namespace2]))
				.then(() => dbFacade.activeNamespacesWithAlias(
					aliasTypeAddress,
					[
						address.stringToAddress(testAddress.three)
					]
				))
				.then(entities => { expect(entities).to.deep.equal([]); })
				.then(() => db.close());
		});

		it('does not return expired namespaces for mosaics', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new NamespaceDb(db);
			const chainHeight = 10;
			const namespace1 = createNamespace(12345, 76756, aliasTypeMosaic, 1, { start: 0, end: 5 });
			const namespace2 = createNamespace(67891, 87823, aliasTypeMosaic, 2, { start: 0, end: 10 });
			const namespace3 = createNamespace(34567, 57231, aliasTypeMosaic, 3, { start: 0, end: 20 });

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(db, 'blocks', [...Array(chainHeight)].map(() => ({}))))
				.then(() => populateCollection(db, 'namespaces', [namespace1, namespace2, namespace3]))
				.then(() => sanitizeDbEntities([namespace1, namespace2, namespace3]))
				.then(() => dbFacade.activeNamespacesWithAlias(aliasTypeMosaic, [76756, 87823, 57231]))
				.then(entities => { expect(entities).to.deep.equal([namespace3]); })
				.then(() => db.close());
		});

		it('does not return expired namespaces for addresses', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new NamespaceDb(db);
			const chainHeight = 10;
			const namespace1 = createNamespace(12345, testAddress.one, aliasTypeAddress, 1, { start: 0, end: 5 });
			const namespace2 = createNamespace(67891, testAddress.two, aliasTypeAddress, 2, { start: 0, end: 10 });
			const namespace3 = createNamespace(34567, testAddress.three, aliasTypeAddress, 3, { start: 0, end: 20 });

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(db, 'blocks', [...Array(chainHeight)].map(() => ({}))))
				.then(() => populateCollection(db, 'namespaces', [namespace1, namespace2, namespace3]))
				.then(() => sanitizeDbEntities([namespace1, namespace2, namespace3]))
				.then(() => dbFacade.activeNamespacesWithAlias(
					aliasTypeAddress,
					[
						address.stringToAddress(testAddress.one),
						address.stringToAddress(testAddress.two),
						address.stringToAddress(testAddress.three)
					]
				))
				.then(entities => { expect(entities).to.deep.equal([namespace3]); })
				.then(() => db.close());
		});
	});

	describe('register namespace transactions by namespace ids', () => {
		const transactionType = catapult.model.EntityType.registerNamespace;
		const createRegisterNamespaceTransaction = (namespaceId, type, name) => ({
			transaction: { type, id: convertToLong(namespaceId), name }
		});

		it('returns register namespace transactions by namespace ids', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new NamespaceDb(db);
			const registerNamespaceTransaction1 = createRegisterNamespaceTransaction(12345, transactionType, 'alias_1');
			const registerNamespaceTransaction2 = createRegisterNamespaceTransaction(67891, transactionType, 'alias_2');
			const registerNamespaceTransaction3 = createRegisterNamespaceTransaction(67891, 123, 'alias_2');

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(
					db,
					'transactions',
					[
						registerNamespaceTransaction1,
						registerNamespaceTransaction2,
						registerNamespaceTransaction3
					]
				))
				.then(() => sanitizeDbEntities([
					registerNamespaceTransaction1, registerNamespaceTransaction2, registerNamespaceTransaction3
				]))
				.then(() => dbFacade.registerNamespaceTransactionsByNamespaceIds([convertToLong(12345), convertToLong(67891)]))
				.then(entities => { expect(entities).to.deep.equal([registerNamespaceTransaction1, registerNamespaceTransaction2]); })
				.then(() => db.close());
		});

		it('returns empty for no register namespace transactions with namespaceId', () => {
			// Arrange:
			const db = new CatapultDb({ networkId: testDbOptions.networkId });
			const dbFacade = new NamespaceDb(db);
			const registerNamespaceTransaction1 = createRegisterNamespaceTransaction(12345, transactionType, 'alias_1');
			const registerNamespaceTransaction2 = createRegisterNamespaceTransaction(67891, transactionType, 'alias_2');

			// Act + Assert:
			return db.connect(testDbOptions.url, 'test')
				.then(() => populateCollection(
					db,
					'transactions',
					[
						registerNamespaceTransaction1,
						registerNamespaceTransaction2
					]
				))
				.then(() => sanitizeDbEntities([
					registerNamespaceTransaction1, registerNamespaceTransaction2
				]))
				.then(() => dbFacade.registerNamespaceTransactionsByNamespaceIds([convertToLong(12121)]))
				.then(entities => { expect(entities).to.deep.equal([]); })
				.then(() => db.close());
		});
	});
});
