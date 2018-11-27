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

/** @module crypto/sha3Hasher */
const arrayUtils = require('../utils/arrayUtils');
const convert = require('../utils/convert');
const { sha3_256, sha3_512 } = require('js-sha3');

const getHasher = (length = 64) => ({ 32: sha3_256, 64: sha3_512 }[length]);

const sha3Hasher = {
	/**
	 * Calculates the hash of data.
	 * @param {Uint8Array} dest The computed hash destination.
	 * @param {Uint8Array} data The data to hash.
	 * @param {numeric} length The hash length in bytes.
	 */
	func: (dest, data, length) => {
		const hasher = getHasher(length);
		const hash = hasher.arrayBuffer(data);
		arrayUtils.copy(dest, arrayUtils.uint8View(hash));
	},

	/**
	 * Creates a hasher object.
	 * @param {numeric} length The hash length in bytes.
	 * @returns {object} The hasher.
	 */
	createHasher: length => {
		let hash;
		return {
			reset: () => {
				hash = getHasher(length).create();
			},
			update: data => {
				if (data instanceof Uint8Array)
					hash.update(data);
				else if ('string' === typeof data)
					hash.update(convert.hexToUint8(data));
				else
					throw Error('unsupported data type');
			},
			finalize: result => {
				arrayUtils.copy(result, arrayUtils.uint8View(hash.arrayBuffer()));
			}
		};
	}
};

module.exports = sha3Hasher;
