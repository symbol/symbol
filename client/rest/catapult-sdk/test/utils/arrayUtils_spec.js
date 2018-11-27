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

const arrayUtils = require('../../src/utils/arrayUtils');
const convert = require('../../src/utils/convert');
const { expect } = require('chai');

describe('array', () => {
	describe('uint8View', () => {
		it('can get uint8 view of array buffer', () => {
			// Arrange:
			const src = convert.hexToUint8('0A12B5675069');

			// Act:
			const view = arrayUtils.uint8View(src.buffer);

			// Assert:
			expect(convert.uint8ToHex(view)).to.equal('0A12B5675069');
		});

		it('can get uint8 view of uint8 typed array', () => {
			// Arrange:
			const src = convert.hexToUint8('0A12B5675069');

			// Act:
			const view = arrayUtils.uint8View(src);

			// Assert:
			expect(convert.uint8ToHex(view)).to.equal('0A12B5675069');
		});

		it('cannot get uint8 view of arbitrary typed array', () => {
			// Arrange:
			const src = new Uint16Array(10);

			// Act:
			expect(() => arrayUtils.uint8View(src)).to.throw('unsupported type passed to uint8View');
		});
	});

	describe('copy', () => {
		it('can copy full typed array', () => {
			// Arrange:
			const src = convert.hexToUint8('0A12B5675069');
			const dest = new Uint8Array(src.length);

			// Act:
			arrayUtils.copy(dest, src);

			// Assert:
			expect(convert.uint8ToHex(dest)).to.equal('0A12B5675069');
		});

		it('can copy partial typed array when dest is same size as src', () => {
			// Arrange:
			const src = convert.hexToUint8('0A12B5675069');
			const dest = new Uint8Array(src.length);

			// Act:
			arrayUtils.copy(dest, src, 3);

			// Assert:
			expect(convert.uint8ToHex(dest)).to.equal('0A12B5000000');
		});

		it('can copy partial typed array when dest is smaller than src', () => {
			// Arrange:
			const src = convert.hexToUint8('0A12B5675069');
			const dest = new Uint8Array(4);

			// Act:
			arrayUtils.copy(dest, src);

			// Assert:
			expect(convert.uint8ToHex(dest)).to.equal('0A12B567');
		});

		it('can copy partial typed array with custom offsets', () => {
			// Arrange:
			const src = convert.hexToUint8('0A12B5675069');
			const dest = new Uint8Array(src.length);

			// Act:
			arrayUtils.copy(dest, src, 3, 2, 1);

			// Assert:
			expect(convert.uint8ToHex(dest)).to.equal('000012B56700');
		});
	});

	describe('isZero', () => {
		it('returns true if typed array is zero', () => {
			// Act:
			const isZero = arrayUtils.isZero(new Uint16Array(10));

			// Assert:
			expect(isZero).to.equal(true);
		});

		const assertIsNonZero = (length, nonZeroOffset) => {
			// Arrange:
			const src = new Uint16Array(length);
			src[nonZeroOffset] = 2;

			// Act
			const isZero = arrayUtils.isZero(src);

			// Assert:
			expect(isZero, `nonzero offset ${nonZeroOffset}`).to.equal(false);
		};

		it('returns false if typed array is non zero', () => {
			// Assert:
			assertIsNonZero(10, 0);
			assertIsNonZero(10, 5);
			assertIsNonZero(10, 9);
		});
	});

	describe('deepEqual', () => {
		it('returns true if typed arrays are equal', () => {
			// Arrange:
			const lhs = convert.hexToUint8('0A12B5675069');
			const rhs = convert.hexToUint8('0A12B5675069');

			// Act:
			const isEqual = arrayUtils.deepEqual(lhs, rhs);

			// Assert:
			expect(isEqual).to.equal(true);
		});

		it('returns false if typed arrays have different sizes', () => {
			// Arrange:
			const shorter = convert.hexToUint8('0A12B5675069');
			const longer = convert.hexToUint8('0A12B567506983');

			// Act:
			const isEqual1 = arrayUtils.deepEqual(shorter, longer);
			const isEqual2 = arrayUtils.deepEqual(longer, shorter);

			// Assert:
			expect(isEqual1).to.equal(false);
			expect(isEqual2).to.equal(false);
		});

		const assertNotEqual = (lhs, unequalOffset) => {
			// Arrange:
			const rhs = new Uint8Array(lhs.length);
			arrayUtils.copy(rhs, lhs);
			rhs[unequalOffset] ^= 0xFF;

			// Act
			const isEqual = arrayUtils.deepEqual(lhs, rhs);

			// Assert:
			expect(isEqual, `unequal offset ${unequalOffset}`).to.equal(false);
		};

		it('returns false if typed arrays are not equal', () => {
			// Arrange:
			const lhs = convert.hexToUint8('0A12B5675069');

			// Assert:
			assertNotEqual(lhs, 0);
			assertNotEqual(lhs, 3);
			assertNotEqual(lhs, 5);
		});

		it('returns true if subset of typed arrays are equal', () => {
			// Arrange: different at 2
			const lhs = convert.hexToUint8('0A12B5675069');
			const rhs = convert.hexToUint8('0A12C5675069');

			// Act:
			const isEqualSubset = arrayUtils.deepEqual(lhs, rhs, 2);
			const isEqualAll = arrayUtils.deepEqual(lhs, rhs);

			// Assert:
			expect(isEqualSubset).to.equal(true);
			expect(isEqualAll).to.equal(false);
		});

		it('returns true if subset of typed arrays of different lengths are equal', () => {
			// Arrange:
			const shorter = convert.hexToUint8('0A12B5');
			const longer = convert.hexToUint8('0A12B567506983');

			// Act:
			const isEqual1 = arrayUtils.deepEqual(shorter, longer, 3);
			const isEqual2 = arrayUtils.deepEqual(longer, shorter, 3);

			// Assert:
			expect(isEqual1).to.equal(true);
			expect(isEqual2).to.equal(true);
		});

		it('returns false if either typed array has fewer elements than requested for comparison', () => {
			// Arrange:
			const shorter = convert.hexToUint8('0A12B5');
			const longer = convert.hexToUint8('0A12B567506983');

			// Act:
			const isEqual1 = arrayUtils.deepEqual(shorter, longer, 4);
			const isEqual2 = arrayUtils.deepEqual(longer, shorter, 4);

			// Assert:
			expect(isEqual1).to.equal(false);
			expect(isEqual2).to.equal(false);
		});
	});
});
