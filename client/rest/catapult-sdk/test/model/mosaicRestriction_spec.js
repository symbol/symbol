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

const mosaicRestriction = require('../../src/model/mosaicRestriction');
const { expect } = require('chai');

describe('mosaic restriction', () => {
	describe('restriction type', () => {
		it('exposes restriction type types', () => {
			// Assert:
			expect(mosaicRestriction.restrictionType).to.deep.equal({
				address: 0,
				global: 1
			});
		});

		it('exposed values are unique', () => {
			// Act:
			const reverseMapping = Object.keys(mosaicRestriction.restrictionType).reduce((state, name) => {
				state[mosaicRestriction.restrictionType[name]] = name;
				return state;
			}, {});

			// Assert:
			expect(Object.keys(mosaicRestriction.restrictionType).length).to.equal(Object.keys(reverseMapping).length);
		});
	});
});
