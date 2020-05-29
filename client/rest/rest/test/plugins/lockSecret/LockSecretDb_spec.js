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

const test = require('./lockSecretDbTestUtils');
const testUtils = require('../../testUtils');
const { expect } = require('chai');

describe('lock secret db', () => {
	const createOwner = testUtils.random.account;

	describe('fetch locks secret by account', () => {
		const addLockSecretByAccountTests = traits => {
			const assertLocks = (dbCallParams, lockGroups) =>
				// Assert:
				test.db.runDbTest(
					traits.collectionName,
					lockGroups.seed,
					db => db[traits.dbMethodName](...dbCallParams),
					locks => {
						// Assert:
						const expectedLocks = lockGroups.expected;
						const expectedIds = expectedLocks.map(lock => lock._id);

						const ids = locks.map(lock => lock._id);
						expect(locks.length).to.equal(expectedLocks.length);
						expect(ids).to.deep.equal(expectedIds);
						expect(locks).to.deep.equal(expectedLocks);
					}
				);

			const createRandomLocks = (startId, count) => {
				const locks = [];
				for (let id = startId; id < startId + count; ++id)
					locks.push(traits.createLockSecret(id, createOwner(), traits.createRandomHash()));
				return locks;
			};

			const ownerToDbApiIds = owner => [traits.toDbApiId(owner)];

			it('returns empty array for account with no locks secret', () => {
				// Arrange: create 3 locks
				const allLocks = traits.createLockSecrets(3, createOwner());

				// Assert:
				return assertLocks([ownerToDbApiIds(createOwner())], {
					seed: allLocks,
					expected: []
				});
			});

			it('returns all locks for single account with locks secret', () => {
				// Arrange: create 10 locks
				const owner = createOwner();
				const seedLocks = traits.createLockSecrets(10, owner);

				// - create additional 5 locks with random owner
				const additionalLocks = createRandomLocks(20, 5);

				// Assert:
				return assertLocks(
					[ownerToDbApiIds(owner)],
					{ seed: seedLocks.concat(additionalLocks), expected: seedLocks.reverse() }
				);
			});

			describe('paging', () => {
				it('query respects supplied document id', () => {
					// Arrange: create 10 locks
					const owner = createOwner();
					const seedLocks = traits.createLockSecrets(10, owner).reverse();
					const expectedLocks = seedLocks.slice(8);

					// Assert:
					return assertLocks(
						[ownerToDbApiIds(owner), seedLocks[7]._id.toString()],
						{ seed: seedLocks, expected: expectedLocks }
					);
				});

				const assertPageSize = (pageSize, expectedSize) => {
					// Arrange: create 200 locks
					const owner = createOwner();
					const seedLocks = traits.createLockSecrets(200, owner);
					const expectedLocks = seedLocks.slice(0, 200).reverse().slice(0, expectedSize);

					// Assert:
					expect(expectedSize).to.equal(expectedLocks.length);
					return assertLocks(
						[ownerToDbApiIds(owner), undefined, pageSize],
						{ seed: seedLocks, expected: expectedLocks }
					);
				};

				// minimum and maximum values are set in CatapultDb ctor
				it('query respects page size', () => assertPageSize(12, 12));
				it('query ensures minimum page size', () => assertPageSize(5, 10));
				it('query ensures maximum page size', () => assertPageSize(150, 100));
			});
		};

		describe('secretLocks by owners', () => {
			describe('by address', () => addLockSecretByAccountTests({
				collectionName: 'secret',
				dbMethodName: 'secretLocksByAccounts',
				createRandomHash: testUtils.random.secret,
				createLockSecret: test.db.createSecretLock,
				createLockSecrets: test.db.createSecretLocks,
				toDbApiId: owner => owner.address
			}));
		});
	});

	describe('fetch individual', () => {
		const addTests = traits => {
			it('returns undefined', () => {
				// Arrange: create lock secret info
				const hash = traits.createRandomHash();
				const lockInfo = traits.createLockSecret(0, createOwner(), hash);

				// Assert:
				return test.db.runDbTest(
					traits.type,
					lockInfo,
					db => db[traits.dbFunctionName](traits.createRandomHash()),
					entity => { expect(entity).to.equal(undefined); }
				);
			});

			it('returns an entity', () => {
				// Arrange: create lock secret info
				const hash = traits.createRandomHash();
				const lockInfo = traits.createLockSecret(0, createOwner(), hash);

				// Assert:
				return test.db.runDbTest(
					traits.type,
					lockInfo,
					db => db[traits.dbFunctionName](hash),
					entity => {
						expect(entity).to.deep.equal(lockInfo);
					}
				);
			});
		};

		describe('secret lock by secret', () => addTests({
			createRandomHash: testUtils.random.secret,
			createLockSecret: test.db.createSecretLock,
			type: 'secret',
			dbFunctionName: 'secretLockBySecret'
		}));
	});
});
