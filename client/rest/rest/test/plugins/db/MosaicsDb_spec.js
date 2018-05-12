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

const { expect } = require('chai');
const test = require('./utils/mosaicDbTestUtils');

describe('mosaics db', () => {
	const createMosaics = (numNamespaces, numMosaicsPerNamespace) => {
		const owner = test.random.publicKey();
		return test.db.createMosaics(owner, numNamespaces, numMosaicsPerNamespace);
	};

	describe('mosaics by ids', () => {
		it('returns empty array for unknown mosaic ids', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011, (inactive) 10000, 10003, 10006, 10009
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[123, 456]]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns single matching mosaic', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011, (inactive) 10000, 10003, 10006, 10009
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0]]),
				entities => { expect(entities).to.deep.equal([mosaics[10]]); }
			);
		});

		it('returns multiple matching mosaics', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011, (inactive) 10000, 10003, 10006, 10009
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0], [10007, 0], [10003, 0]]),
				entities => { expect(entities).to.deep.equal([mosaics[10], mosaics[7], mosaics[3]]); }
			);
		});

		it('returns only known mosaics', () => {
			// Arrange: mosaic ids: 10000, 10001, ... 10011, (inactive) 10000, 10003, 10006, 10009
			const mosaics = createMosaics(3, 4);

			// Assert:
			return test.db.runDbTest(
				mosaics,
				db => db.mosaicsByIds([[10010, 0], [10021, 0], [10003, 0]]),
				entities => expect(entities).to.deep.equal([mosaics[10], mosaics[3]])
			);
		});
	});

	describe('mosaics by namespace id', () => {
		const assertMosaics = (dbCallParams, allMosaics, expectedMosaics) => {
			// Arrange:
			const dbEntities = allMosaics;

			// Assert:
			return test.db.runDbTest(
				dbEntities,
				db => db.mosaicsByNamespaceId(...dbCallParams),
				mosaics => {
					// Assert:
					const ids = mosaics.map(mosaic => mosaic._id);
					const expectedIds = expectedMosaics.map(mosaic => mosaic._id);
					expect(mosaics.length).to.equal(expectedMosaics.length);
					expect(ids).to.deep.equal(expectedIds);
					expect(mosaics).to.deep.equal(expectedMosaics);
				}
			);
		};

		it('for namespace with no mosaics', () => {
			// Arrange: create 4 namespaces, 3 mosaics per namespace followed by 1 inactive mosaic
			// namespaces: 20000, ... 20003
			const mosaics = createMosaics(4, 3);
			const expectedMosaics = [];

			// Assert:
			const namespaceId = [20004, 0];
			return assertMosaics([namespaceId], mosaics, expectedMosaics);
		});

		it('for namespace with mosaics', () => {
			// Arrange: create 4 namespaces, 3 mosaics per namespace followed by 1 inactive mosaic
			// mosaic ids: 10000, 10001, ... 10011, (inactive) 10000, 10003, 10006, 10009
			// slice picks mosaics from namespace 20002: 10006, 1007, 10008
			const mosaics = createMosaics(4, 3);
			const expectedMosaics = mosaics.slice(6, 9).reverse();

			// Assert:
			const namespaceId = [20002, 0];
			return assertMosaics([namespaceId], mosaics, expectedMosaics);
		});

		it('query respects supplied document id', () => {
			// Arrange: 10 namespaces, 10 mosaics per namespace followed by 1 inactive mosaic
			// mosaic ids: 10000, 10001, ... 10099, (inactive) 10000, 10010, 10020, ... 10090
			// mosaics are in namespaces: 20000, 20001, ..., 20009
			// slice picks mosaics from namespace 20001: 10010, 10011, 10012
			const mosaics = createMosaics(10, 10);
			const expectedMosaics = mosaics.slice(10, 12).reverse();

			// Assert: get mosaics from namespace 20001 with mosaic ids below that of 12th
			const namespaceId = [20001, 0];
			return assertMosaics([namespaceId, mosaics[12]._id.toString()], mosaics, expectedMosaics);
		});

		const assertPageSize = (pageSize, expectedSize) => {
			// Arrange: create 100 mosaics in single namespace
			// mosaic ids: 10000, 10001, ... 10099, (inactive) 10000
			// slice picks mosaics: 10000, 10001, ... 10099 (without last mosaic that is inactive)
			const mosaics = createMosaics(1, 100);
			const expectedMosaics = mosaics.slice(0, 100).reverse().slice(0, expectedSize);

			// Assert:
			const namespaceId = [20000, 0];
			return assertMosaics([namespaceId, undefined, pageSize], mosaics, expectedMosaics);
		};

		// minimum and maximum values are set in CatapultDb ctor
		it('query respects page size', () => assertPageSize(12, 12));
		it('query ensures minimum page size', () => assertPageSize(5, 10));
		it('query ensures maximum page size', () => assertPageSize(150, 100));
	});
});
