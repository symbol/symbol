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

const charMapping = require('./charMapping');

const Char_To_Nibble_Map = (() => {
	const builder = charMapping.createBuilder();
	builder.addRange('0', '9', 0);
	builder.addRange('a', 'f', 10);
	builder.addRange('A', 'F', 10);
	return builder.map;
})();

const Char_To_Digit_Map = (() => {
	const builder = charMapping.createBuilder();
	builder.addRange('0', '9', 0);
	return builder.map;
})();

const Nibble_To_Char_Map = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'];

const tryParseByte = (char1, char2) => {
	const nibble1 = Char_To_Nibble_Map[char1];
	const nibble2 = Char_To_Nibble_Map[char2];
	return undefined === nibble1 || undefined === nibble2
		? undefined
		: (nibble1 << 4) | nibble2;
};

/** @exports utils/convert */
const convert = {
	/**
	 * Decodes two hex characters into a byte.
	 * @param {string} char1 The first hex digit.
	 * @param {string} char2 The second hex digit.
	 * @returns {numeric} The decoded byte.
	 */
	toByte: (char1, char2) => {
		const byte = tryParseByte(char1, char2);
		if (undefined === byte)
			throw Error(`unrecognized hex char '${char1}${char2}'`);

		return byte;
	},

	/**
	 * Determines whether or not a string is a hex string.
	 * @param {string} input The string to test.
	 * @returns {boolean} true if the input is a hex string, false otherwise.
	 */
	isHexString: input => {
		if (0 !== input.length % 2)
			return false;

		for (let i = 0; i < input.length; i += 2) {
			if (undefined === tryParseByte(input[i], input[i + 1]))
				return false;
		}

		return true;
	},

	/**
	 * Converts a hex string to a uint8 array.
	 * @param {string} input A hex encoded string.
	 * @returns {Uint8Array} A uint8 array corresponding to the input.
	 */
	hexToUint8: input => {
		if (0 !== input.length % 2)
			throw Error(`hex string has unexpected size '${input.length}'`);

		const output = new Uint8Array(input.length / 2);
		for (let i = 0; i < input.length; i += 2)
			output[i / 2] = convert.toByte(input[i], input[i + 1]);

		return output;
	},

	/**
	 * Converts a uint8 array to a hex string.
	 * @param {Uint8Array} input A uint8 array.
	 * @returns {string} A hex encoded string corresponding to the input.
	 */
	uint8ToHex: input => {
		let s = '';
		input.forEach(byte => {
			s += Nibble_To_Char_Map[byte >> 4];
			s += Nibble_To_Char_Map[byte & 0x0F];
		});

		return s;
	},

	/**
	 * Tries to parse a string representing an unsigned integer.
	 * @param {string} str The string to parse.
	 * @returns {numeric} The number represented by the input or undefined.
	 */
	tryParseUint: str => {
		if ('0' === str)
			return 0;

		let value = 0;
		for (let i = 0; i < str.length; ++i) {
			const char = str[i];
			const digit = Char_To_Digit_Map[char];
			if (undefined === digit || (0 === value && 0 === digit))
				return undefined;

			value *= 10;
			value += digit;

			if (value > Number.MAX_SAFE_INTEGER)
				return undefined;
		}

		return value;
	},

	/**
	 * Converts a uint8 array to a uint32 array.
	 * @param {Uint8Array} input A uint8 array.
	 * @returns {Uint32Array} A uint32 array created from the input.
	 */
	uint8ToUint32: input => new Uint32Array(input.buffer),

	/**
	 * Converts a uint32 array to a uint8 array.
	 * @param {Uint32Array} input A uint32 array.
	 * @returns {Uint8Array} A uint8 array created from the input.
	 */
	uint32ToUint8: input => new Uint8Array(input.buffer),

	/** Converts an unsigned byte to a signed byte with the same binary representation.
	 * @param {Numeric} input An unsigned byte.
	 * @returns {Numeric} A signed byte with the same binary representation as the input.
	 */
	uint8ToInt8: input => {
		if (0xFF < input)
			throw Error(`input '${input}' is out of range`);

		return input << 24 >> 24;
	},

	/** Converts a signed byte to an unsigned byte with the same binary representation.
	 * @param {Numeric} input A signed byte.
	 * @returns {Numeric} An unsigned byte with the same binary representation as the input.
	 */
	int8ToUint8: input => {
		if (127 < input || -128 > input)
			throw Error(`input '${input}' is out of range`);

		return input & 0xFF;
	}
};

module.exports = convert;
