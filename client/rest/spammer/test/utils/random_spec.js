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

const random = require('../../src/utils/random');
const { expect } = require('chai');

describe('random', () => {
	const bigNumbers = [0x0100000000, 0x0100000001, 0x123456789, Number.MAX_SAFE_INTEGER];

	describe('uint32', () => {
		it('respects limits', () => {
			// Arrange:
			const max = 1234;
			const values = [];

			// Act:
			for (let i = 0; 10 > i; ++i)
				values.push(random.uint32(max));

			// Assert:
			values.forEach(value => {
				expect(value).to.be.within(0, max);
			});
		});

		it('accepts max uint32', () => {
			// Arrange:
			const max = 0xFFFFFFFF;

			// Act:
			const value = random.uint32(max);

			// Assert:
			expect(value).to.be.within(0, max);
		});

		it('throws if param does not fit into 32 bit', () => {
			// Assert:
			bigNumbers.forEach(value => {
				expect(() => { random.uint32(value); }, `${value} is too large`).to.throw(`${value} does not fit into 32 bits`);
			});
		});
	});
});
