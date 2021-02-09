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

const namespace = require('../../src/model/namespace');
const convert = require('../../src/utils/convert');
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

	describe('encodeNamespace', () => {
		it('Testnet valid', () => {
			// Assert:
			const encoded = namespace.encodeNamespace(convert.hexToUint8('C0FB8AA409916260'), 152);
			const encodedHex = convert.uint8ToHex(encoded);
			const expectedEncodedHex = '9960629109A48AFBC0000000000000000000000000000000';
			expect(encodedHex).to.be.equal(expectedEncodedHex);
			expect(encoded).to.be.deep.equal(convert.hexToUint8(expectedEncodedHex));
		});
	});
});
