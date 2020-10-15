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

const CatapultDb = require('../../../src/db/CatapultDb');
const SecretLocksDb = require('../../../src/plugins/lockSecret/LockSecretDb');
const test = require('../../db/utils/dbTestUtils');
const { expect } = require('chai');
const sinon = require('sinon');

describe('secret locks db', () => {
	const ownerAddressTest1 = test.random.address();
	const ownerAddressTest2 = test.random.address();
	const ownerAddressTest3 = test.random.address();
	const secretTest01 = test.random.hash();
	const secretTest02 = test.random.hash();

	const { createObjectId } = test.db;

	const runSecretLocksDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) =>
		test.db.runDbTest(dbEntities, 'secretLocks', db => new SecretLocksDb(db), issueDbCommand, assertDbCommandResult);

	const createSecretLock = (objectId, ownerAddress, secret) => ({
		_id: createObjectId(objectId),
		lock: {
			ownerAddress: ownerAddress ? Buffer.from(ownerAddress) : undefined,
			mosaicId: '',
			amount: '',
			endHeight: '',
			status: '',
			hashAlgorithm: '',
			secret: secret || undefined,
			recipientAddress: '',
			compositeHash: ''
		}
	});

	describe('secretLocks', () => {
		const paginationOptions = {
			pageSize: 10,
			pageNumber: 1,
			sortField: 'id',
			sortDirection: -1
		};

		const runTestAndVerifyIds = (dbSecretLocks, dbQuery, expectedIds) => {
			const expectedObjectIds = expectedIds.map(id => createObjectId(id));

			return runSecretLocksDbTest(
				dbSecretLocks,
				dbQuery,
				page => {
					const returnedIds = page.data.map(t => t.id);
					expect(page.data.length).to.equal(expectedObjectIds.length);
					expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
				}
			);
		};

		it('returns expected structure', () => {
			// Arrange:
			const dbSecretLocks = [createSecretLock(10, ownerAddressTest1)];

			// Act + Assert:
			return runSecretLocksDbTest(
				dbSecretLocks,
				db => db.secretLocks([ownerAddressTest1], undefined, paginationOptions),
				page => {
					const expected_keys = ['id', 'lock'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
				}
			);
		});

		it('returns empty array for unknown address', () => {
			// Arrange:
			const dbSecretLocks = [
				createSecretLock(10, ownerAddressTest1),
				createSecretLock(20, ownerAddressTest1)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbSecretLocks, db => db.secretLocks([ownerAddressTest2], undefined, paginationOptions), []);
		});

		it('returns filtered lock secret by owner address', () => {
			// Arrange:
			const dbSecretLocks = [
				createSecretLock(10, ownerAddressTest1),
				createSecretLock(20, ownerAddressTest2)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbSecretLocks, db => db.secretLocks([ownerAddressTest2], undefined, paginationOptions), [20]);
		});

		it('returns filtered lock secret by owner address, multiple addresses', () => {
			// Arrange:
			const dbSecretLocks = [
				createSecretLock(10, ownerAddressTest1),
				createSecretLock(20, ownerAddressTest2),
				createSecretLock(30, ownerAddressTest3)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbSecretLocks,
				db => db.secretLocks([ownerAddressTest1, ownerAddressTest2], undefined, paginationOptions),
				[10, 20]
			);
		});

		it('returns filtered lock secret by secret', () => {
			// Arrange:
			const dbSecretLocks = [
				createSecretLock(10, ownerAddressTest1, secretTest01),
				createSecretLock(20, ownerAddressTest2, secretTest02)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbSecretLocks, db => db.secretLocks([ownerAddressTest2], secretTest02, paginationOptions), [20]);
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbSecretLocks = () => [
				createSecretLock(10, ownerAddressTest1),
				createSecretLock(20, ownerAddressTest1),
				createSecretLock(30, ownerAddressTest1)
			];

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runSecretLocksDbTest(
					dbSecretLocks(),
					db => db.secretLocks([ownerAddressTest1], undefined, options),
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
				return runSecretLocksDbTest(
					dbSecretLocks(),
					db => db.secretLocks([ownerAddressTest1], undefined, options),
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
				return runSecretLocksDbTest(
					dbSecretLocks(),
					db => db.secretLocks([ownerAddressTest1], undefined, options),
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
			const dbSecretLocks = () => [
				createSecretLock(10, ownerAddressTest1),
				createSecretLock(20, ownerAddressTest1),
				createSecretLock(30, ownerAddressTest1)
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
				return runTestAndVerifyIds(dbSecretLocks(), db => db.secretLocks([ownerAddressTest1], undefined, options), [30]);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(dbSecretLocks(), db => db.secretLocks([ownerAddressTest1], undefined, options), [10]);
			});
		});
	});
});
