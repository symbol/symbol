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

/** @module crypto/keyPair */
const convert = require('../utils/convert');
const tweetnacl = require('tweetnacl');

// region exported functions

/**
 * A catapult public key.
 * @typedef {Uint8Array} PublicKey
 */

/**
 * A catapult key pair composed of a public, a private, and a secret keys.
 * @typedef {object} KeyPair
 * @property {module:crypto/keyPair~PublicKey} publicKey Public key.
 * @property {Uint8Array} privateKey Private key.
 * @property {Uint8Array} secretKey Secret key.
 */

const keyPairModule = {
	/**
 	 * Creates a key pair from a private key string.
	 * @param {string} privateKeyString A hex encoded private key string.
	 * @returns {module:crypto/keyPair~KeyPair} Key pair.
	 */
	createKeyPairFromPrivateKeyString: privateKeyString => {
		const privateKey = convert.hexToUint8(privateKeyString);
		if (32 !== privateKey.length)
			throw Error(`private key has unexpected size: ${privateKey.length}`);

		return Object.assign({ privateKey }, tweetnacl.sign.keyPair.fromSeed(privateKey));
	},

	/**
	 * Signs a data buffer with a key pair.
	 * @param {module:crypto/keyPair~KeyPair} keyPair Key pair to use for signing.
	 * @param {Uint8Array} data Data to sign.
	 * @returns {Uint8Array} Signature.
	 */
	sign: (keyPair, data) => tweetnacl.sign.detached(data, keyPair.secretKey),

	/**
	 * Verifies a signature.
	 * @param {module:crypto/keyPair~PublicKey} publicKey Public key to use for verification.
	 * @param {Uint8Array} data Data to verify.
	 * @param {Uint8Array} signature Signature to verify.
	 * @returns {boolean} true if the signature is verifiable, false otherwise.
	 */
	verify: (publicKey, data, signature) => tweetnacl.sign.detached.verify(data, signature, publicKey)
};

// endregion

module.exports = keyPairModule;
