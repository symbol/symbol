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

const CmcDb = require('../../../src/plugins/cmc/CmcDb');
const test = require('../../db/utils/dbTestUtils');
const { expect } = require('chai');
const MongoDb = require('mongodb');

const { Binary, Long } = MongoDb;

describe('cmc db', () => {
	const { createObjectId } = test.db;

	const runCMCDbTest = (dbEntities, issueDbCommand, assertDbCommandResult) =>
		test.db.runDbTest(dbEntities, 'mosaics', db => new CmcDb(db), issueDbCommand, assertDbCommandResult);

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
			return runCMCDbTest(
				mosaics,
				db => db.mosaicsByIds([[123, 456]]),
				entities => { expect(entities).to.deep.equal([]); }
			);
		});

		it('returns single matching mosaic', () => {
			// Arrange:
			const mosaics = createMosaics(3, 4);

			// Assert:
			return runCMCDbTest(
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
			return runCMCDbTest(
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
			return runCMCDbTest(
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
