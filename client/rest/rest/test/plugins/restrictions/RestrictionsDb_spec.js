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

const test = require('./restrictionsDbTestUtils');
const CatapultDb = require('../../../src/db/CatapultDb');
const { convertToLong } = require('../../../src/db/dbUtils');
const RestrictionsDb = require('../../../src/plugins/restrictions/RestrictionsDb');
const dbTestUtils = require('../../db/utils/dbTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

describe('restrictions db', () => {
	describe('account restrictions', () => {
		const removeMongoId = entity => {
			delete entity.meta.id;
			return entity;
		};

		it('returns undefined for unknown account', () => {
			// Arrange:
			const { address } = test.random.account();
			const accountRestrictions1 = test.accountDb.createAccountRestrictions(
				address, { numAddresses: 3, numMosaics: 3, numOperations: 3 }
			);

			// Assert:
			return test.accountDb.runDbTest(
				accountRestrictions1,
				db => db.accountRestrictionsByAddresses([[123, 456]]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns found empty account restrictions for single account', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const randomAddress2 = test.random.account().address;
			const accountRestrictions1 = test.accountDb.createAccountRestrictions(
				randomAddress1,
				{ numAddresses: 0, numMosaics: 0, numOperations: 0 }
			);
			const accountRestrictions2 = test.accountDb.createAccountRestrictions(
				address,
				{ numAddresses: 0, numMosaics: 0, numOperations: 0 }
			);
			const accountRestrictions3 = test.accountDb.createAccountRestrictions(
				randomAddress2,
				{ numAddresses: 0, numMosaics: 0, numOperations: 0 }
			);

			// Assert:
			return test.accountDb.runDbTest(
				[accountRestrictions1, accountRestrictions2, accountRestrictions3],
				db => db.accountRestrictionsByAddresses([address]),
				entities => { expect(entities).to.deep.equal([removeMongoId(accountRestrictions2)]); }
			);
		});

		it('returns found populated account restrictions for single account', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const randomAddress2 = test.random.account().address;
			const accountRestrictions1 = test.accountDb.createAccountRestrictions(
				randomAddress1,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);
			const accountRestrictions2 = test.accountDb.createAccountRestrictions(
				address,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);
			const accountRestrictions3 = test.accountDb.createAccountRestrictions(
				randomAddress2,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);

			// Assert:
			return test.accountDb.runDbTest(
				[accountRestrictions1, accountRestrictions2, accountRestrictions3],
				db => db.accountRestrictionsByAddresses([address]),
				entities => { expect(entities[0]).to.deep.equal(removeMongoId(accountRestrictions2)); }
			);
		});

		it('returns found populated account restrictions for multiple accounts', () => {
			// Arrange:
			const { address } = test.random.account();
			const randomAddress1 = test.random.account().address;
			const accountRestrictions1 = test.accountDb.createAccountRestrictions(
				address,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);
			const accountRestrictions2 = test.accountDb.createAccountRestrictions(
				randomAddress1,
				{ numAddresses: 3, numMosaics: 6, numOperations: 2 }
			);
			const accountRestrictions3 = test.accountDb.createAccountRestrictions(
				address,
				{ numAddresses: 1, numMosaics: 4, numOperations: 3 }
			);

			// Assert:
			return test.accountDb.runDbTest(
				[accountRestrictions1, accountRestrictions2, accountRestrictions3],
				db => db.accountRestrictionsByAddresses([address]),
				entities => { expect(entities).to.deep.equal([removeMongoId(accountRestrictions1), removeMongoId(accountRestrictions3)]); }
			);
		});
	});

	describe('mosaic restrictions', () => {
		const { address } = catapult.model;
		const { createObjectId } = dbTestUtils.db;
		const testAddress1 = address.stringToAddress('SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ');
		const testAddress2 = address.stringToAddress('NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA');

		const paginationOptions = {
			pageSize: 10,
			pageNumber: 1,
			sortField: 'id',
			sortDirection: -1
		};

		const createMosaicRestriction = (objectId, mosaicId, entryType, targetAddress) => ({
			_id: createObjectId(objectId),
			mosaicRestrictionEntry: {
				compositeHash: '',
				entryType,
				mosaicId: mosaicId ? convertToLong(mosaicId) : undefined,
				targetAddress: targetAddress ? Buffer.from(targetAddress) : undefined,
				restrictions: []
			}
		});

		const runMosaicRestrictionsDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) =>
			dbTestUtils.db.runDbTest(dbEntities, 'mosaicRestrictions', db => new RestrictionsDb(db), issueDbCommand, assertDbCommandResult);

		const runTestAndVerifyIds = (dbRestrictions, dbQuery, expectedIds) => {
			const expectedObjectIds = expectedIds.map(id => createObjectId(id));

			return runMosaicRestrictionsDbTest(
				dbRestrictions,
				dbQuery,
				mosaicRestrictionsPage => {
					const returnedIds = mosaicRestrictionsPage.data.map(t => t.id);
					expect(mosaicRestrictionsPage.data.length).to.equal(expectedObjectIds.length);
					expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
				}
			);
		};

		it('returns expected structure', () => {
			// Arrange:
			const dbMosaicRestrictions = [createMosaicRestriction(10)];

			// Act + Assert:
			return runMosaicRestrictionsDbTest(
				dbMosaicRestrictions,
				db => db.mosaicRestrictions(undefined, undefined, undefined, paginationOptions),
				page => {
					const expected_keys = ['id', 'mosaicRestrictionEntry'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
				}
			);
		});

		it('returns filtered mosaic restrictions by mosaicId', () => {
			// Arrange:
			const dbMosaicRestrictions = [
				createMosaicRestriction(10, [0xAAAD29AA, 0xAAC67FAA]),
				createMosaicRestriction(20, [0x1CAD29E3, 0x0DC67FBE])
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMosaicRestrictions,
				db => db.mosaicRestrictions([0xAAAD29AA, 0xAAC67FAA], undefined, undefined, paginationOptions), [10]
			);
		});

		it('returns filtered mosaic restrictions by entry type', () => {
			// Arrange:
			const dbMosaicRestrictions = [
				createMosaicRestriction(10, undefined, 0),
				createMosaicRestriction(20, undefined, 1)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMosaicRestrictions,
				db => db.mosaicRestrictions(undefined, 0, undefined, paginationOptions), [10]
			);
		});

		it('returns filtered mosaic restrictions by targetAddress', () => {
			// Arrange:
			const dbMosaicRestrictions = [
				createMosaicRestriction(10, undefined, undefined, testAddress1),
				createMosaicRestriction(20, undefined, undefined, testAddress2)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMosaicRestrictions,
				db => db.mosaicRestrictions(undefined, undefined, testAddress2, paginationOptions), [20]
			);
		});

		it('returns all mosaic restrictions if no filters provided', () => {
			// Arrange:
			const dbMosaicRestrictions = [
				createMosaicRestriction(10),
				createMosaicRestriction(20),
				createMosaicRestriction(30)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMosaicRestrictions,
				db => db.mosaicRestrictions(undefined, undefined, undefined, paginationOptions), [10, 20, 30]
			);
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbMosaicRestrictions = () => [
				createMosaicRestriction(10),
				createMosaicRestriction(20),
				createMosaicRestriction(30)
			];

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runMosaicRestrictionsDbTest(
					dbMosaicRestrictions(),
					db => db.mosaicRestrictions(undefined, undefined, undefined, options),
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
				return runMosaicRestrictionsDbTest(
					dbMosaicRestrictions(),
					db => db.mosaicRestrictions(undefined, undefined, undefined, options),
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
				return runMosaicRestrictionsDbTest(
					dbMosaicRestrictions(),
					db => db.mosaicRestrictions(undefined, undefined, undefined, options),
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
			const dbMosaicRestrictions = () => [
				createMosaicRestriction(10),
				createMosaicRestriction(20),
				createMosaicRestriction(30)
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
				return runTestAndVerifyIds(
					dbMosaicRestrictions(), db => db.mosaicRestrictions(undefined, undefined, undefined, options), [30]
				);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(
					dbMosaicRestrictions(), db => db.mosaicRestrictions(undefined, undefined, undefined, options), [10]
				);
			});
		});
	});
});
