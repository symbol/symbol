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

import idReducer from '../../../src/catapult-sdk/model/idReducer.js';
import { expect } from 'chai';

describe('id reducer', () => {
	describe('id to name lookup', () => {
		// region basic

		it('can be built around empty tuples', () => {
			// Act:
			const lookup = idReducer.createIdToNameLookup([]);
			const name = lookup.findName(2n);

			// Assert:
			expect(lookup.length).to.equal(0);
			expect(name).to.equal(undefined);
		});

		it('can be built arround 1-level tuples', () => {
			// Act:
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: 11n, parentId: 0n },
				{ name: 'bob', namespaceId: 25n, parentId: 0n },
				{ name: 'carol', namespaceId: 37n, parentId: 0n }
			]);
			const names = {
				alice: lookup.findName(11n),
				bob: lookup.findName(25n),
				carol: lookup.findName(37n)
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
				{ name: 'alice', namespaceId: 11n, parentId: 0n },
				{ name: 'apple', namespaceId: 2n, parentId: 11n },
				{ name: 'banana', namespaceId: 4n, parentId: 11n },
				{ name: 'bob', namespaceId: 25n, parentId: 0n },
				{ name: 'carrot', namespaceId: 6n, parentId: 25n }
			]);
			const names = {
				alice: lookup.findName(11n),
				apple: lookup.findName(2n),
				banana: lookup.findName(4n),
				bob: lookup.findName(25n),
				carrot: lookup.findName(6n)
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
				{ name: 'alice', namespaceId: 11n, parentId: 0n },
				{ name: 'apple', namespaceId: 2n, parentId: 11n },
				{ name: 'mac', namespaceId: 3n, parentId: 2n },
				{ name: 'red', namespaceId: 23n, parentId: 3n },
				{ name: 'banana', namespaceId: 4n, parentId: 11n }
			]);
			const names = {
				alice: lookup.findName(11n),
				apple: lookup.findName(2n),
				mac: lookup.findName(3n),
				red: lookup.findName(23n),
				banana: lookup.findName(4n)
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
				{ name: 'alice', namespaceId: 11n, parentId: 0n }
			]);

			// Act:
			const name = lookup.findName(2n);

			// Assert:
			expect(name).to.equal(undefined);
		});

		it('gives first conflicting tuple preference', () => {
			// Arrange: apple has two conflicting parents
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: 11n, parentId: 0n },
				{ name: 'apple', namespaceId: 2n, parentId: 11n },
				{ name: 'bob', namespaceId: 25n, parentId: 0n },
				{ name: 'apple', namespaceId: 2n, parentId: 25n }
			]);

			// Act:
			const name = lookup.findName(2n);

			// Assert: the first definition wins
			expect(name).to.equal('alice.apple');
		});

		it('returns undefined when any parent id is unknown', () => {
			// Arrange:
			const lookup = idReducer.createIdToNameLookup([
				{ name: 'alice', namespaceId: 11n, parentId: 20n }
			]);

			// Act:
			const name = lookup.findName(11n);

			// Assert:
			expect(name).to.equal(undefined);
		});

		// endregion
	});
});
