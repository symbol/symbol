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

const test = require('./namespaceDbTestUtils');
const CatapultDb = require('../../../src/db/CatapultDb');
const dbUtils = require('../../../src/db/dbUtils');
const AccountType = require('../../../src/plugins/AccountType');
const NamespaceDb = require('../../../src/plugins/namespace/NamespaceDb');
const testDbOptions = require('../../db/utils/testDbOptions');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');

const { address } = catapult.model;
const { convertToLong } = dbUtils;
const { Binary } = MongoDb;

describe('namespace db', () => {
	const createOwner = test.random.account;

	describe('namespace by id', () => {
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
					expect(entity.namespace.depth).to.equal(1);
					expect(entity).to.deep.equal(namespaces[3]);
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
					expect(entity.namespace.depth).to.equal(2);
					expect(entity).to.deep.equal(namespaces[1]);
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
					expect(entity.namespace.depth).to.equal(3);
					expect(entity).to.deep.equal(namespaces[2]);
				}
			);
		});
	});

	const assertNamespaces = (dbCallParams, namespaceGroups) =>
		// Assert:
		test.db.runDbTest(
			namespaceGroups.seed,
			db => db.namespacesByOwners(...dbCallParams),
			namespaces => {
				// Assert:
				const expectedNamespaces = namespaceGroups.expected;
				const expectedIds = expectedNamespaces.map(namespace => namespace._id);

				const ids = namespaces.map(namespace => namespace._id);
				expect(namespaces.length).to.equal(expectedNamespaces.length);
				expect(ids).to.deep.equal(expectedIds);
				expect(namespaces).to.deep.equal(expectedNamespaces);
			}
		);

	const createRandomNamespaces = (startId, count) => {
		const namespaces = [];
		for (let id = startId; id < startId + count; ++id) {
			const lifetime = { start: 10 * id, end: 10 * (id + 1) };
			namespaces.push(test.db.createNamespace(id, createOwner(), 0, id, [1234], lifetime, false, { type: 0 }));
		}

		return namespaces;
	};

	const addNamespaceByOwnersTests = traits => {
		const ownerToDbApiIds = owner => [traits.toDbApiId(owner)];

		it('returns empty array for account with no namespaces', () => {
			// Arrange: create 3 namespaces and 3 inactive namespaces
			const allNamespaces = test.db.createNamespaces(3, createOwner());

			// Assert:
			return traits.assertNamespaces([traits.type, ownerToDbApiIds(createOwner())], { seed: allNamespaces, expected: [] });
		});

		it('returns all namespaces for single account with namespaces', () => {
			// Arrange: create 10 namespaces and 10 inactive namespaces with known owner
			const owner = createOwner();
			const seedNamespaces = test.db.createNamespaces(10, owner);
			const activeNamespaces = seedNamespaces.slice(0, 10).reverse();

			// - create additional 5 namespace with random owner
			const additionalNamespaces = createRandomNamespaces(20, 5);

			// Assert:
			return traits.assertNamespaces(
				[traits.type, ownerToDbApiIds(owner)],
				{ seed: seedNamespaces.concat(additionalNamespaces), expected: activeNamespaces }
			);
		});

		describe('paging', () => {
			it('query respects supplied document id', () => {
				// Arrange: create 10 namespaces and 10 inactive namespaces
				const owner = createOwner();
				const seedNamespaces = test.db.createNamespaces(10, owner);
				const activeNamespaces = seedNamespaces.slice(0, 10).reverse();
				const expectedNamespaces = activeNamespaces.slice(8);

				// Assert:
				return traits.assertNamespaces(
					[traits.type, ownerToDbApiIds(owner), activeNamespaces[7]._id.toString()],
					{ seed: seedNamespaces, expected: expectedNamespaces }
				);
			});

			const assertPageSize = (pageSize, expectedSize) => {
				// Arrange: create 200 namespaces and 200 inactive namespaces
				const owner = createOwner();
				const seedNamespaces = test.db.createNamespaces(200, owner);
				const expectedNamespaces = seedNamespaces.slice(0, 200).reverse().slice(0, expectedSize);

				// Assert:
				expect(expectedSize).to.equal(expectedNamespaces.length);
				return traits.assertNamespaces(
					[traits.type, ownerToDbApiIds(owner), undefined, pageSize],
					{ seed: seedNamespaces, expected: expectedNamespaces }
				);
			};

			// minimum and maximum values are set in CatapultDb ctor
			it('query respects page size', () => assertPageSize(12, 12));
			it('query ensures minimum page size', () => assertPageSize(5, 10));
			it('query ensures maximum page size', () => assertPageSize(150, 100));
		});

		const runMultiOwnerTest = createAccountIds => {
			// Arrange: create 5 namespaces and 5 inactive namespaces with known owner
			const owner1 = createOwner();
			const seedNamespaces1 = test.db.createNamespaces(5, owner1);
			const activeNamespaces1 = seedNamespaces1.slice(0, 5);

			// - create 3 namespaces and 3 inactive namespaces with (other) known owner
			const owner2 = createOwner();
			const seedNamespaces2 = test.db.createNamespaces(3, owner2, 10);
			const activeNamespaces2 = seedNamespaces2.slice(0, 3);

			// - create additional 5 namespace with random owner
			const additionalNamespaces = createRandomNamespaces(16, 5);

			// Assert:
			return assertNamespaces([traits.type, createAccountIds(owner1, owner2)], {
				seed: seedNamespaces1.concat(additionalNamespaces, seedNamespaces2),
				expected: activeNamespaces1.concat(activeNamespaces2).reverse()
			});
		};

		it(
			'returns all namespaces for multiple accounts with namespaces',
			() => runMultiOwnerTest((owner1, owner2) => [owner1, owner2].map(traits.toDbApiId))
		);

		it(
			'returns all namespaces for multiple accounts with namespaces and ignores accounts with no namespaces ',
			() => runMultiOwnerTest((owner1, owner2) =>
				[owner1, createOwner(), createOwner(), owner2].map(traits.toDbApiId))
		);
	};

	describe('namespaces by owners', () => {
		describe('by publicKey', () => addNamespaceByOwnersTests({
			assertNamespaces,
			type: AccountType.publicKey,
			toDbApiId: owner => owner.publicKey
		}));

		describe('by address', () => addNamespaceByOwnersTests({
			assertNamespaces,
			type: AccountType.address,
			toDbApiId: owner => owner.address
		}));
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
			one: 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV',
			two: 'SCFZFP7N5C3P6EHP5D2UJ7GQD7Q7ZIENV4NZ6ELN',
			three: 'SAAM2O7SSJ2A7AU3DZJMSTTRFZT5TFDPQ3ZIIJX7'
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
