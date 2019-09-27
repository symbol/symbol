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

const metadata = require('../../src/model/metadata');
const { expect } = require('chai');

describe('metadata', () => {
	describe('metadata type', () => {
		it('exposes metadata types', () => {
			// Assert:
			expect(metadata.metadataType).to.deep.equal({
				account: 0,
				mosaic: 1,
				namespace: 2
			});
		});

		it('exposed values are unique', () => {
			// Act:
			const reverseMapping = Object.keys(metadata.metadataType).reduce((state, name) => {
				state[metadata.metadataType[name]] = name;
				return state;
			}, {});

			// Assert:
			expect(Object.keys(metadata.metadataType).length).to.equal(Object.keys(reverseMapping).length);
		});
	});
});
