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

const restriction = require('../../src/model/restriction');
const { expect } = require('chai');

describe('restriction', () => {
	describe('mosaic restriction', () => {
		describe('mosaic restriction type', () => {
			it('exposes mosaic restriction type types', () => {
				// Assert:
				expect(restriction.mosaicRestriction.restrictionType).to.deep.equal({
					address: 0,
					global: 1
				});
			});

			it('exposed values are unique', () => {
				// Act:
				const reverseMapping = Object.keys(restriction.mosaicRestriction.restrictionType).reduce((state, name) => {
					state[restriction.mosaicRestriction.restrictionType[name]] = name;
					return state;
				}, {});

				// Assert:
				expect(Object.keys(restriction.mosaicRestriction.restrictionType).length).to.equal(Object.keys(reverseMapping).length);
			});
		});
	});
});
