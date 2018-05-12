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

/** @module utils/array */

const arrayUtils = {
	/**
	 * Creates a Uint8Array view on top of input.
	 * @param {ArrayBuffer|Uint8Array} input The input array.
	 * @returns {Uint8Array} A Uint8Array view on top of input.
	 */
	uint8View: input => {
		if (ArrayBuffer === input.constructor)
			return new Uint8Array(input); // note that wrapping an ArrayBuffer in an Uint8Array does not make a copy
		else if (Uint8Array === input.constructor)
			return input;

		throw Error('unsupported type passed to uint8View');
	},

	/**
	 * Copies elements from a source array to a destination array.
	 * @param {Array} dest The destination array.
	 * @param {Array} src The source array.
	 * @param {numeric} [numElementsToCopy=undefined] The number of elements to copy.
	 * @param {numeric} [destOffset=0] The first index of the destination to write.
	 * @param {numeric} [srcOffset=0] The first index of the source to read.
	 */
	copy: (dest, src, numElementsToCopy, destOffset = 0, srcOffset = 0) => {
		const length = undefined === numElementsToCopy ? dest.length : numElementsToCopy;
		for (let i = 0; i < length; ++i)
			dest[destOffset + i] = src[srcOffset + i];
	},

	/**
	 * Determines whether or not an array is zero-filled.
	 * @param {Array} array The array to check.
	 * @returns {boolean} true if the array is zero-filled, false otherwise.
	 */
	isZero: array => array.every(value => 0 === value),

	/**
	 * Deeply checks the equality of two arrays.
	 * @param {Array} lhs First array to compare.
	 * @param {Array} rhs Second array to compare.
	 * @param {numeric} [numElementsToCompare=undefined] The number of elements to compare.
	 * @returns {boolean} true if all compared elements are equal, false otherwise.
	 */
	deepEqual: (lhs, rhs, numElementsToCompare) => {
		let length = numElementsToCompare;
		if (undefined === length) {
			if (lhs.length !== rhs.length)
				return false;

			({ length } = lhs);
		}

		if (length > lhs.length || length > rhs.length)
			return false;

		for (let i = 0; i < length; ++i) {
			if (lhs[i] !== rhs[i])
				return false;
		}

		return true;
	}
};

module.exports = arrayUtils;
