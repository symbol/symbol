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
const { convertToLong } = require('../../../src/db/dbUtils');
const MetadataDb = require('../../../src/plugins/metadata/MetadataDb');
const test = require('../../db/utils/dbTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;

describe('metadata db', () => {
	const { createObjectId } = test.db;

	const runMetadataDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) =>
		test.db.runDbTest(dbEntities, 'metadata', db => new MetadataDb(db), issueDbCommand, assertDbCommandResult);

	describe('metadata', () => {
		const testAddress1 = address.stringToAddress('SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ');
		const testAddress2 = address.stringToAddress('NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA');

		const paginationOptions = {
			pageSize: 10,
			pageNumber: 1,
			sortField: 'id',
			sortDirection: -1
		};

		const createMetadata = (objectId, sourceAddress, targetAddress, scopedMetadataKey, targetId, metadataType) => ({
			_id: createObjectId(objectId),
			metadataEntry: {
				sourceAddress: sourceAddress ? Buffer.from(sourceAddress) : undefined,
				targetAddress: targetAddress ? Buffer.from(targetAddress) : undefined,
				scopedMetadataKey: scopedMetadataKey ? convertToLong(scopedMetadataKey) : undefined,
				targetId: targetId ? convertToLong(targetId) : undefined,
				metadataType: undefined !== metadataType ? metadataType : undefined
			}
		});

		const runTestAndVerifyIds = (dbMetadata, dbQuery, expectedIds) => {
			const expectedObjectIds = expectedIds.map(id => createObjectId(id));

			return runMetadataDbTest(
				dbMetadata,
				dbQuery,
				metadataPage => {
					const returnedIds = metadataPage.data.map(t => t.id);
					expect(metadataPage.data.length).to.equal(expectedObjectIds.length);
					expect(returnedIds.sort()).to.deep.equal(expectedObjectIds.sort());
				}
			);
		};

		it('returns expected structure', () => {
			// Arrange:
			const dbMetadata = [createMetadata(10)];

			// Act + Assert:
			return runMetadataDbTest(
				dbMetadata,
				db => db.metadata(undefined, undefined, undefined, undefined, undefined, paginationOptions),
				page => {
					const expected_keys = ['id', 'metadataEntry'];
					expect(Object.keys(page.data[0]).sort()).to.deep.equal(expected_keys.sort());
				}
			);
		});

		it('returns filtered metadata by sourceAddress', () => {
			// Arrange:
			const dbMetadata = [
				createMetadata(10, testAddress1),
				createMetadata(20, testAddress2)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMetadata,
				db => db.metadata(testAddress2, undefined, undefined, undefined, undefined, paginationOptions), [20]
			);
		});

		it('returns filtered metadata by targetAddress', () => {
			// Arrange:
			const dbMetadata = [
				createMetadata(10, undefined, testAddress1),
				createMetadata(20, undefined, testAddress2)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMetadata,
				db => db.metadata(undefined, testAddress2, undefined, undefined, undefined, paginationOptions), [20]
			);
		});

		it('returns filtered metadata by scopedMetadataKey', () => {
			// Arrange:
			const dbMetadata = [
				createMetadata(10, undefined, undefined, [0x1CAD29E3, 0x0DC67FBE]),
				createMetadata(20, undefined, undefined, [0xAAAD29AA, 0xAAC67FAA])
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMetadata,
				db => db.metadata(undefined, undefined, [0x1CAD29E3, 0x0DC67FBE], undefined, undefined, paginationOptions), [10]
			);
		});

		it('returns filtered metadata by targetId', () => {
			// Arrange:
			const dbMetadata = [
				createMetadata(10, undefined, undefined, undefined, [0xAAAD29AA, 0xAAC67FAA]),
				createMetadata(20, undefined, undefined, undefined, [0x1CAD29E3, 0x0DC67FBE])
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMetadata,
				db => db.metadata(undefined, undefined, undefined, [0xAAAD29AA, 0xAAC67FAA], undefined, paginationOptions), [10]
			);
		});

		it('returns filtered metadata by metadataType', () => {
			// Arrange:
			const dbMetadata = [
				createMetadata(10, undefined, undefined, undefined, undefined, 45),
				createMetadata(20, undefined, undefined, undefined, undefined, 87)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMetadata,
				db => db.metadata(undefined, undefined, undefined, undefined, 87, paginationOptions), [20]
			);
		});

		it('returns all the metadata if no filters provided', () => {
			// Arrange:
			const dbMetadata = [
				createMetadata(10),
				createMetadata(20)
			];

			// Act + Assert:
			return runTestAndVerifyIds(
				dbMetadata,
				db => db.metadata(undefined, undefined, undefined, undefined, undefined, paginationOptions), [10, 20]
			);
		});

		describe('respects sort conditions', () => {
			// Arrange:
			const dbMetadata = () => [
				createMetadata(10),
				createMetadata(20),
				createMetadata(30)
			];

			it('direction ascending', () => {
				const options = {
					pageSize: 10,
					pageNumber: 1,
					sortField: 'id',
					sortDirection: 1
				};

				// Act + Assert:
				return runMetadataDbTest(
					dbMetadata(),
					db => db.metadata(undefined, undefined, undefined, undefined, undefined, options),
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
				return runMetadataDbTest(
					dbMetadata(),
					db => db.metadata(undefined, undefined, undefined, undefined, undefined, options),
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
				return runMetadataDbTest(
					dbMetadata(),
					db => db.metadata(undefined, undefined, undefined, undefined, undefined, options),
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
			const dbMetadata = () => [
				createMetadata(10),
				createMetadata(20),
				createMetadata(30)
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
					dbMetadata(), db => db.metadata(undefined, undefined, undefined, undefined, undefined, options), [30]
				);
			});

			it('lt', () => {
				options.sortDirection = -1;

				// Act + Assert:
				return runTestAndVerifyIds(
					dbMetadata(), db => db.metadata(undefined, undefined, undefined, undefined, undefined, options), [10]
				);
			});
		});
	});
});
