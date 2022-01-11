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

const arrayUtils = require('../utils/arrayUtils');
const base32 = require('../utils/base32');
const convert = require('../utils/convert');
const jsSha3 = require('js-sha3');
const Ripemd160 = require('ripemd160');

const constants = {
	sizes: {
		ripemd160: 20,
		addressDecoded: 24,
		addressEncoded: 39,
		key: 32,
		checksum: 3
	}
};

/** @exports model/address */
const address = {
	/**
	 * Converts an encoded address string to a decoded address.
	 * @param {string} encoded Encoded address string.
	 * @returns {Uint8Array} Decoded address corresponding to the input.
	 */

	stringToAddress: encoded => {
		if (constants.sizes.addressEncoded !== encoded.length)
			throw Error(`${encoded} does not represent a valid encoded address`);

		// base32 module cannot manage arbitrary size conversion so some ugly temporary padding is required
		return base32.decode(`${encoded}A`).subarray(0, constants.sizes.addressDecoded);
	},

	/**
	 * Converts a decoded address to an encoded address string.
	 * @param {Uint8Array} decoded Decoded address.
	 * @returns {string} Encoded address string corresponding to the input.
	 */
	addressToString: decoded => {
		if (constants.sizes.addressDecoded !== decoded.length)
			throw Error(`${convert.uint8ToHex(decoded)} does not represent a valid decoded address`);

		// base32 module cannot manage arbitrary size conversion so some ugly temporary padding is required
		const paddedDecodedAddress = new Uint8Array(constants.sizes.addressDecoded + 1);
		paddedDecodedAddress.set(decoded);
		return base32.encode(paddedDecodedAddress).slice(0, -1);
	},

	/**
	 * Converts a public key to a decoded address for a specific network.
	 * @param {module:crypto/keyPair~PublicKey} publicKey Public key.
	 * @param {numeric} networkIdentifier Network identifier.
	 * @returns {Uint8Array} Decoded address corresponding to the inputs.
	 */
	publicKeyToAddress: (publicKey, networkIdentifier) => {
		// step 1: sha3 hash of the public key
		const publicKeyHash = jsSha3.sha3_256.arrayBuffer(publicKey);

		// step 2: ripemd160 hash of (1)
		const ripemdHash = new Ripemd160().update(Buffer.from(publicKeyHash)).digest();

		// step 3: add network identifier byte in front of (2)
		const decodedAddress = new Uint8Array(constants.sizes.addressDecoded);
		decodedAddress[0] = networkIdentifier;
		arrayUtils.copy(decodedAddress, ripemdHash, constants.sizes.ripemd160, 1);

		// step 4: concatenate (3) and the checksum of (3)
		const hash = jsSha3.sha3_256.arrayBuffer(decodedAddress.subarray(0, constants.sizes.ripemd160 + 1));
		arrayUtils.copy(decodedAddress, arrayUtils.uint8View(hash), constants.sizes.checksum, constants.sizes.ripemd160 + 1);

		return decodedAddress;
	},

	/**
	 * Determines the validity of a decoded address.
	 * @param {Uint8Array} decoded Decoded address.
	 * @returns {boolean} true if the decoded address is valid, false otherwise.
	 */
	isValidAddress: decoded => {
		const hash = jsSha3.sha3_256.create();
		const checksumBegin = constants.sizes.addressDecoded - constants.sizes.checksum;
		hash.update(decoded.subarray(0, checksumBegin));
		const checksum = new Uint8Array(constants.sizes.checksum);
		arrayUtils.copy(checksum, arrayUtils.uint8View(hash.arrayBuffer()), constants.sizes.checksum);
		return arrayUtils.deepEqual(checksum, decoded.subarray(checksumBegin));
	},

	/**
	 * Determines the validity of an encoded address string.
	 * @param {string} encoded Encoded address string.
	 * @returns {boolean} true if the encoded address string is valid, false otherwise.
	 */
	isValidEncodedAddress: encoded => {
		if (constants.sizes.addressEncoded !== encoded.length)
			return false;

		try {
			const decoded = address.stringToAddress(encoded);
			const paddedDecoded = base32.decode(`${encoded}A`);

			// when decoding an address we will always get a valid address, however, when encoding an address the result may not always be
			// correct, thus a two-way check is required to see that the original address was valid (padded with 0)
			return address.isValidAddress(decoded) && 0 === paddedDecoded[constants.sizes.addressDecoded];
		} catch (err) {
			return false;
		}
	}
};

module.exports = address;
