/** @module utils/uint64 */

import convert from './convert';

/**
 * An exact uint64 representation composed of two 32bit values.
 * @typedef {Array} uint64
 * @property {numeric} 0 The low 32bit value.
 * @property {numeric} 1 The high 32bit value.
 */

export default {
	/**
	 * Tries to compact a uint64 into a simple numeric.
	 * @param {module:utils/uint64~uint64} uint64 A uint64 value.
	 * @returns {numeric|module:utils/uint64~uint64}
	 * A numeric if the uint64 is no greater than Number.MAX_SAFE_INTEGER or the original uint64 value otherwise.
	 */
	compact: uint64 => {
		const low = uint64[0];
		const high = uint64[1];

		// don't compact if the value is >= 2^53
		if (0x00200000 <= high)
			return uint64;

		// multiply because javascript bit operations operate on 32bit values
		return (high * 0x100000000) + low;
	},

	/**
	 * Parses a hex string into a uint64.
	 * @param {string} input A hex encoded string.
	 * @returns {module:utils/uint64~uint64} The uint64 representation of the input.
	 */
	fromHex: input => {
		if (16 !== input.length)
			throw Error(`hex string has unexpected size '${input.length}'`);

		let hexString = input;
		if (16 > hexString.length)
			hexString = '0'.repeat(16 - hexString.length) + hexString;

		const uint8Array = convert.hexToUint8(hexString);
		const view = new DataView(uint8Array.buffer);
		return [view.getUint32(4), view.getUint32(0)];
	},

	/**
	 * Converts a uint64 into a hex string.
	 * @param {module:utils/uint64~uint64} uint64 A uint64 value.
	 * @returns {string} A hex encoded string representing the uint64.
	 */
	toHex: uint64 => {
		const uint32Array = new Uint32Array(uint64);
		const uint8Array = convert.uint32ToUint8(uint32Array).reverse();
		return convert.uint8ToHex(uint8Array);
	},

	/**
	 * Returns true if a uint64 is zero.
	 * @param {module:utils/uint64~uint64} uint64 A uint64 value.
	 * @returns {boolean} true if the value is zero.
	 */
	isZero: uint64 => 0 === uint64[0] && 0 === uint64[1]
};
