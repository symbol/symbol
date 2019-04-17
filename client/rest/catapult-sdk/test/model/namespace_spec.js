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

const namespace = require('../../src/model/namespace');
const { expect } = require('chai');

describe('namespace', () => {
	describe('alias type', () => {
		it('exposes alias type types', () => {
			// Assert:
			expect(namespace.aliasType).to.deep.equal({
				mosaic: 1,
				address: 2
			});
		});

		it('exposed values are unique', () => {
			// Act:
			const reverseMapping = Object.keys(namespace.aliasType).reduce((state, name) => {
				state[namespace.aliasType[name]] = name;
				return state;
			}, {});

			// Assert:
			expect(Object.keys(namespace.aliasType).length).to.equal(Object.keys(reverseMapping).length);
		});
	});
});
