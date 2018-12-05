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

const test = require('./utils/mosaicDbTestUtils');
const { expect } = require('chai');

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
});
