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

const serialize = require('./serialize');
const sizes = require('./sizes');
const crypto = require('../crypto/keyPair');
const sha3Hasher = require('../crypto/sha3Hasher');

// serialize.toBuffer returns a nodejs buffer that does not copy any data when slicing
const serializeToBuffer = (codec, transaction) => serialize.toBuffer(codec, transaction).slice(sizes.transactionHeader);

const transactionExtensions = {
	/**
	 * Calculates the hash of a transaction.
	 * @param {module:modelBinary/ModelCodec} codec Transaction codec.
	 * @param {object} transaction Transaction to hash.
	 * @returns {Uint8Array} Transaction hash.
	 */
	hash: (codec, transaction) => {
		const transactionHash = new Uint8Array(32);
		const hasher = sha3Hasher.createHasher(32);
		hasher.reset();

		// "R"
		hasher.update(transaction.signature.slice(0, 32));

		// pubkey
		hasher.update(transaction.signerPublicKey);

		// data
		const transactionBuffer = serializeToBuffer(codec, transaction);
		hasher.update(transactionBuffer);
		hasher.finalize(transactionHash);

		return transactionHash;
	},

	/**
	 * Signs a transaction using a signer's private key.
	 * @param {module:modelBinary/ModelCodec} codec Transaction codec.
	 * @param {object} keyPair Signer's key pair.
	 * @param {object} transaction Transaction to sign.
	 */
	sign: (codec, keyPair, transaction) => {
		const transactionBuffer = serializeToBuffer(codec, transaction);
		transaction.signature = crypto.sign(keyPair, transactionBuffer);
	},

	/**
	 * Verifies the signature of a transaction.
	 * @param {module:modelBinary/ModelCodec} codec Transaction codec.
	 * @param {object} transaction Transaction to verify.
	 * @returns {boolean} true if the transaction signature is valid.
	 */
	verify: (codec, transaction) => {
		const transactionBuffer = serializeToBuffer(codec, transaction);
		return crypto.verify(transaction.signerPublicKey, transactionBuffer, transaction.signature);
	}
};

module.exports = transactionExtensions;
