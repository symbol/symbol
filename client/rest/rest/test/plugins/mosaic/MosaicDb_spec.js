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
const MosaicDb = require('../../../src/plugins/mosaic/MosaicDb');
const test = require('../../db/utils/dbTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { Binary, Long } = MongoDb;
const { address } = catapult.model;

describe('mosaic db', () => {
	const { createObjectId } = test.db;

	const runMosaicsDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) =>
		test.db.runDbTest(dbEntities, 'mosaics', db => new MosaicDb(db), issueDbCommand, assertDbCommandResult);

	describe('mosaics', () => {
		const ownerAddressTest1 = address.stringToAddress('SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ');
		const ownerAddressTest2 = address.stringToAddress('NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA');

		const paginationOptions = {
			pageSize: 10,
			pageNumber: 1,
			sortField: 'id',
			sortDirection: -1
		};

		const createMosaic = (objectId, mosaicId, ownerAddress) => ({
			_id: createObjectId(objectId),
			mosaic: { id: mosaicId, ownerAddress: ownerAddress ? Buffer.from(ownerAddress) : undefined }
		});

		const runTestAndVerifyIds = (dbMosaics, dbQuery, expectedIds) => {
			const expectedObjectIds = expectedIds.map(id => createObjectId(id));

			return runMosaicsDbTest(
				dbMosaics,
				dbQuery,
				mosaicsPage => {
					const returnedIds = mosaicsPage.data.map(t => t.id);
					expect(mosaicsPage.data.length).to.equal(expectedObjectIds.length);
					expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
				}
			);
		};

		it('returns expected structure', () => {
			// Arrange:
			const dbMosaics = [createMosaic(10, 100, ownerAddressTest1)];

			// Act + Assert:
			return runMosaicsDbTest(
				dbMosaics,
				db => db.mosaics(undefined, paginationOptions),
				page => {
					const expected_keys = ['id', 'mosaic'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
				}
			);
		});

		it('returns empty array for unknown ownerAddress', () => {
			// Arrange:
			const dbMosaics = [
				createMosaic(10, 1, ownerAddressTest1),
				createMosaic(20, 2, ownerAddressTest1)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbMosaics, db => db.mosaics(ownerAddressTest2, paginationOptions), []);
		});

		it('returns filtered mosaics by ownerAddress', () => {
			// Arrange:
			const dbMosaics = [
				createMosaic(10, 1, ownerAddressTest1),
				createMosaic(20, 2, ownerAddressTest2)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbMosaics, db => db.mosaics(ownerAddressTest2, paginationOptions), [20]);
		});

		it('returns all the mosaics if no ownerAddress provided', () => {
			// Arrange:
			const dbMosaics = [
				createMosaic(10, 1, ownerAddressTest1),
				createMosaic(20, 2, ownerAddressTest2)
			];

			// Act + Assert:
			return runTestAndVerifyIds(dbMosaics, db => db.mosaics(undefined, paginationOptions), [10, 20]);
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbMosaics = () => [
				createMosaic(10, 20),
				createMosaic(20, 30),
				createMosaic(30, 10)
			];

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runMosaicsDbTest(
					dbMosaics(),
					db => db.mosaics(undefined, options),
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
				return runMosaicsDbTest(
					dbMosaics(),
					db => db.mosaics(undefined, options),
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
				return runMosaicsDbTest(
					dbMosaics(),
					db => db.mosaics(undefined, options),
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
			const dbMosaics = () => [
				createMosaic(10, 20),
				createMosaic(20, 30),
				createMosaic(30, 10)
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
				return runTestAndVerifyIds(dbMosaics(), db => db.mosaics(undefined, options), [30]);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(dbMosaics(), db => db.mosaics(undefined, options), [10]);
			});
		});
	});

	describe('mosaics by ids', () => {
		const createMosaic = (id, mosaicId, ownerAddress, parentId) => {
			const mosaic = {
				ownerAddress: new Binary(ownerAddress),
				id: Long.fromNumber(mosaicId),
				namespaceId: Long.fromNumber(parentId)
			};

			return { _id: createObjectId(id), mosaic };
		};

		/*
		 * Creates mosaics with ids in the 1000s range, whereas namespace ids will be in the 2000s range
		 */
		const createMosaics = (numNamespaces, numMosaicsPerNamespace) => {
			const ownerAddress = test.random.address();
			const mosaics = [];
			let dbId = 0;
			let id = 10000;
			for (let namespaceId = 0; namespaceId < numNamespaces; ++namespaceId) {
				for (let i = 0; i < numMosaicsPerNamespace; ++i)
					mosaics.push(createMosaic(dbId++, id++, ownerAddress, 20000 + namespaceId));
			}

			return mosaics;
		};

		it('returns empty array for unknown mosaic ids', () => {
			// Arrange:
			const mosaics = createMosaics(3, 4);

			// Assert:
			return runMosaicsDbTest(
				mosaics,
				db => db.mosaicsByIds([[123, 456]]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns single matching mosaic', () => {
			// Arrange:
			const mosaics = createMosaics(3, 4);

			// Assert:
			return runMosaicsDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0]]),
				entities => {
					expect(entities).to.deep.equal([{ id: createObjectId(10), ...mosaics[10] }]);
				}
			);
		});

		it('returns multiple matching mosaics', () => {
			// Arrange:
			const mosaics = createMosaics(3, 4);

			// Assert:
			return runMosaicsDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0], [10007, 0], [10003, 0]]),
				entities => {
					expect(entities).to.deep.equal([
						{ id: createObjectId(10), ...mosaics[10] },
						{ id: createObjectId(7), ...mosaics[7] },
						{ id: createObjectId(3), ...mosaics[3] }
					]);
				}
			);
		});

		it('returns only known mosaics', () => {
			// Arrange:
			const mosaics = createMosaics(3, 4);

			// Assert:
			return runMosaicsDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0], [10021, 0], [10003, 0]]),
				entities => expect(entities).to.deep.equal([
					{ id: createObjectId(10), ...mosaics[10] },
					{ id: createObjectId(3), ...mosaics[3] }
				])
			);
		});
	});
});
