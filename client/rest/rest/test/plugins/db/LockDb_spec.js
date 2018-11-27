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
const test = require('./utils/lockDbTestUtils');
const testUtils = require('../../testUtils');
const { expect } = require('chai');

describe('lock db', () => {
	const createOwner = testUtils.random.account;

	describe('fetch locks by account', () => {
		const addLockByAccountTests = traits => {
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
					locks.push(traits.createLockInfo(id, createOwner(), traits.createRandomHash()));
				return locks;
			};

			const ownerToDbApiIds = owner => [traits.toDbApiId(owner)];

			it('returns empty array for account with no locks', () => {
				// Arrange: create 3 locks
				const allLocks = traits.createLockInfos(3, createOwner());

				// Assert:
				return assertLocks([traits.type, ownerToDbApiIds(createOwner())], {
					seed: allLocks,
					expected: []
				});
			});

			it('returns all locks for single account with locks', () => {
				// Arrange: create 10 locks
				const owner = createOwner();
				const seedLocks = traits.createLockInfos(10, owner);

				// - create additional 5 locks with random owner
				const additionalLocks = createRandomLocks(20, 5);

				// Assert:
				return assertLocks(
					[traits.type, ownerToDbApiIds(owner)],
					{ seed: seedLocks.concat(additionalLocks), expected: seedLocks.reverse() }
				);
			});

			describe('paging', () => {
				it('query respects supplied document id', () => {
					// Arrange: create 10 locks
					const owner = createOwner();
					const seedLocks = traits.createLockInfos(10, owner).reverse();
					const expectedLocks = seedLocks.slice(8);

					// Assert:
					return assertLocks(
						[traits.type, ownerToDbApiIds(owner), seedLocks[7]._id.toString()],
						{ seed: seedLocks, expected: expectedLocks }
					);
				});

				const assertPageSize = (pageSize, expectedSize) => {
					// Arrange: create 200 locks
					const owner = createOwner();
					const seedLocks = traits.createLockInfos(200, owner);
					const expectedLocks = seedLocks.slice(0, 200).reverse().slice(0, expectedSize);

					// Assert:
					expect(expectedSize).to.equal(expectedLocks.length);
					return assertLocks(
						[traits.type, ownerToDbApiIds(owner), undefined, pageSize],
						{ seed: seedLocks, expected: expectedLocks }
					);
				};

				// minimum and maximum values are set in CatapultDb ctor
				it('query respects page size', () => assertPageSize(12, 12));
				it('query ensures minimum page size', () => assertPageSize(5, 10));
				it('query ensures maximum page size', () => assertPageSize(150, 100));
			});

			const runMultiOwnerTest = createAccountIds => {
				// Arrange: create 5 locks and 5 inactive locks with known owner
				const owner1 = createOwner();
				const seedLocks1 = traits.createLockInfos(5, owner1);
				const activeLocks1 = seedLocks1.slice(0, 5);

				// - create 3 locks and 3 inactive locks with (other) known owner
				const owner2 = createOwner();
				const seedLocks2 = traits.createLockInfos(3, owner2, 10);
				const activeLocks2 = seedLocks2.slice(0, 3);

				// - create additional 5 locks with random owner
				const additionalLocks = createRandomLocks(16, 5);

				// Assert:
				return assertLocks([traits.type, createAccountIds(owner1, owner2)], {
					seed: seedLocks1.concat(additionalLocks, seedLocks2),
					expected: activeLocks1.concat(activeLocks2).reverse()
				});
			};

			it(
				'returns all hashLocks for multiple accounts with hashLocks',
				() => runMultiOwnerTest((owner1, owner2) => [owner1, owner2].map(traits.toDbApiId))
			);

			it(
				'returns all hashLocks for multiple accounts with hashLocks and ignores accounts with no hashLocks ',
				() => runMultiOwnerTest((owner1, owner2) =>
					[owner1, createOwner(), createOwner(), owner2].map(traits.toDbApiId))
			);
		};

		describe('hashLocks by owners', () => {
			describe('by publicKey', () => addLockByAccountTests({
				collectionName: 'hash',
				dbMethodName: 'hashLocksByAccounts',
				createRandomHash: testUtils.random.hash,
				createLockInfo: test.db.createHashLockInfo,
				createLockInfos: test.db.createHashLockInfos,
				type: AccountType.publicKey,
				toDbApiId: owner => owner.publicKey
			}));

			describe('by address', () => addLockByAccountTests({
				collectionName: 'hash',
				dbMethodName: 'hashLocksByAccounts',
				createRandomHash: testUtils.random.hash,
				createLockInfo: test.db.createHashLockInfo,
				createLockInfos: test.db.createHashLockInfos,
				type: AccountType.address,
				toDbApiId: owner => owner.address
			}));
		});

		describe('secretLocks by owners', () => {
			describe('by publicKey', () => addLockByAccountTests({
				collectionName: 'secret',
				dbMethodName: 'secretLocksByAccounts',
				createRandomHash: testUtils.random.secret,
				createLockInfo: test.db.createSecretLockInfo,
				createLockInfos: test.db.createSecretLockInfos,
				type: AccountType.publicKey,
				toDbApiId: owner => owner.publicKey
			}));

			describe('by address', () => addLockByAccountTests({
				collectionName: 'secret',
				dbMethodName: 'secretLocksByAccounts',
				createRandomHash: testUtils.random.secret,
				createLockInfo: test.db.createSecretLockInfo,
				createLockInfos: test.db.createSecretLockInfos,
				type: AccountType.address,
				toDbApiId: owner => owner.address
			}));
		});
	});

	describe('fetch individual', () => {
		const addTests = traits => {
			it('returns undefined', () => {
				// Arrange: create lock info
				const hash = traits.createRandomHash();
				const lockInfo = traits.createLockInfo(0, createOwner(), hash);

				// Assert:
				return test.db.runDbTest(
					traits.type,
					lockInfo,
					db => db[traits.dbFunctionName](traits.createRandomHash()),
					entity => { expect(entity).to.equal(undefined); }
				);
			});

			it('returns an entity', () => {
				// Arrange: create lock info
				const hash = traits.createRandomHash();
				const lockInfo = traits.createLockInfo(0, createOwner(), hash);

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

		describe('hash lock by hash', () => addTests({
			createRandomHash: testUtils.random.hash,
			createLockInfo: test.db.createHashLockInfo,
			type: 'hash',
			dbFunctionName: 'hashLockByHash'
		}));

		describe('secret lock by secret', () => addTests({
			createRandomHash: testUtils.random.secret,
			createLockInfo: test.db.createSecretLockInfo,
			type: 'secret',
			dbFunctionName: 'secretLockBySecret'
		}));
	});
});
