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

const idReducer = require('../../src/model/idReducer');
const { expect } = require('chai');

describe('id reducer', () => {
	describe('id to name lookup', () => {
		// region basic

		it('can be built around empty tuples', () => {
			// Act:
			const lookup = idReducer.createIdToNameLookup([]);
			const name = lookup.findName([0, 2]);

			// Assert:
			expect(lookup.length).to.equal(0);
			expect(name).to.equal(undefined);
		});

		it('can be built arround 1-level tuples', () => {
			// Act:
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: [1, 1], parentId: [0, 0] },
				{ name: 'bob', namespaceId: [2, 5], parentId: [0, 0] },
				{ name: 'carol', namespaceId: [3, 7], parentId: [0, 0] }
			]);
			const names = {
				alice: lookup.findName([1, 1]),
				bob: lookup.findName([2, 5]),
				carol: lookup.findName([3, 7])
			};

			// Assert:
			expect(lookup.length).to.equal(3);
			expect(names).to.deep.equal({
				alice: 'alice',
				bob: 'bob',
				carol: 'carol'
			});
		});

		it('can be built arround 2-level tuples', () => {
			// Act:
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: [1, 1], parentId: [0, 0] },
				{ name: 'apple', namespaceId: [0, 2], parentId: [1, 1] },
				{ name: 'banana', namespaceId: [0, 4], parentId: [1, 1] },
				{ name: 'bob', namespaceId: [2, 5], parentId: [0, 0] },
				{ name: 'carrot', namespaceId: [0, 6], parentId: [2, 5] }
			]);
			const names = {
				alice: lookup.findName([1, 1]),
				apple: lookup.findName([0, 2]),
				banana: lookup.findName([0, 4]),
				bob: lookup.findName([2, 5]),
				carrot: lookup.findName([0, 6])
			};

			// Assert:
			expect(lookup.length).to.equal(5);
			expect(names).to.deep.equal({
				alice: 'alice',
				apple: 'alice.apple',
				banana: 'alice.banana',
				bob: 'bob',
				carrot: 'bob.carrot'
			});
		});

		it('can be built arround multilevel tuples', () => {
			// Act:
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: [1, 1], parentId: [0, 0] },
				{ name: 'apple', namespaceId: [0, 2], parentId: [1, 1] },
				{ name: 'mac', namespaceId: [0, 3], parentId: [0, 2] },
				{ name: 'red', namespaceId: [2, 3], parentId: [0, 3] },
				{ name: 'banana', namespaceId: [0, 4], parentId: [1, 1] }
			]);
			const names = {
				alice: lookup.findName([1, 1]),
				apple: lookup.findName([0, 2]),
				mac: lookup.findName([0, 3]),
				red: lookup.findName([2, 3]),
				banana: lookup.findName([0, 4])
			};

			// Assert:
			expect(lookup.length).to.equal(5);
			expect(names).to.deep.equal({
				alice: 'alice',
				apple: 'alice.apple',
				mac: 'alice.apple.mac',
				red: 'alice.apple.mac.red',
				banana: 'alice.banana'
			});
		});

		// endregion

		// region edge cases

		it('returns undefined when id is unknown', () => {
			// Arrange:
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: [1, 1], parentId: [0, 0] }
			]);

			// Act:
			const name = lookup.findName([0, 2]);

			// Assert:
			expect(name).to.equal(undefined);
		});

		it('gives first conflicting tuple preference', () => {
			// Arrange: apple has two conflicting parents
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: [1, 1], parentId: [0, 0] },
				{ name: 'apple', namespaceId: [0, 2], parentId: [1, 1] },
				{ name: 'bob', namespaceId: [2, 5], parentId: [0, 0] },
				{ name: 'apple', namespaceId: [0, 2], parentId: [2, 5] }
			]);

			// Act:
			const name = lookup.findName([0, 2]);

			// Assert: the first definition wins
			expect(name).to.equal('alice.apple');
		});

		it('returns undefined when any parent id is unknown', () => {
			// Arrange:
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: [1, 1], parentId: [2, 0] }
			]);

			// Act:
			const name = lookup.findName([1, 1]);

			// Assert:
			expect(name).to.equal(undefined);
		});

		// endregion
	});
});
