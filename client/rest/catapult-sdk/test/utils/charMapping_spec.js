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

const charMapping = require('../../src/utils/charMapping');
const { expect } = require('chai');

describe('char mapping', () => {
	describe('builder', () => {
		it('initially has empty map', () => {
			// Arrange:
			const builder = charMapping.createBuilder();

			// Act:
			const { map } = builder;

			// Assert:
			expect(map).to.deep.equal({});
		});

		it('can add single arbitrary range with zero base', () => {
			// Arrange:
			const builder = charMapping.createBuilder();

			// Act:
			builder.addRange('d', 'f', 0);
			const { map } = builder;

			// Assert:
			expect(map).to.deep.equal({
				d: 0,
				e: 1,
				f: 2
			});
		});

		it('can add single arbitrary range with nonzero base', () => {
			// Arrange:
			const builder = charMapping.createBuilder();

			// Act:
			builder.addRange('d', 'f', 17);
			const { map } = builder;

			// Assert:
			expect(map).to.deep.equal({
				d: 17,
				e: 18,
				f: 19
			});
		});

		it('can add multiple arbitrary ranges', () => {
			// Arrange:
			const builder = charMapping.createBuilder();

			// Act:
			builder.addRange('b', 'b', 8);
			builder.addRange('d', 'f', 17);
			builder.addRange('y', 'z', 0);
			const { map } = builder;

			// Assert:
			expect(map).to.deep.equal({
				b: 8,
				d: 17,
				e: 18,
				f: 19,
				y: 0,
				z: 1
			});
		});

		it('can add multiple arbitrary overlapping ranges', () => {
			// Arrange:
			const builder = charMapping.createBuilder();

			// Act:
			builder.addRange('b', 'b', 18);
			builder.addRange('d', 'f', 17);
			builder.addRange('y', 'z', 19);
			const { map } = builder;

			// Assert:
			expect(map).to.deep.equal({
				b: 18,
				d: 17,
				e: 18,
				f: 19,
				y: 19,
				z: 20
			});
		});
	});
});
