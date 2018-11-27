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

const AccountType = require('../../../src/plugins/AccountType');
const test = require('./utils/namespaceDbTestUtils');
const { expect } = require('chai');

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
			namespaces.push(test.db.createNamespace(id, createOwner(), 0, id, [1234], lifetime, false));
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
});
