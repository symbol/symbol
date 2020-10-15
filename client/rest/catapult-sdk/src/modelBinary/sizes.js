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

/** @module modelBinary/sizes */

const sizes = {
	/**
	 * @property {numeric} Size of a signature.
	 */
	signature: 64,

	/**
	 * @property {numeric} Size of a signer public key.
	 */
	signerPublicKey: 32,

	/**
	 * @property {numeric} Size of a decoded address.
	 */
	addressDecoded: 24,

	/**
	 * @property {numeric} Size of a transaction header.
	 */
	transactionHeader: 4 + 4 + 64 + 32 + 4,

	/**
	 * @property {numeric} Size of a sha3 256 hash.
	 */
	hash256: 32,

	/**
	 * @property {numeric} Size of a sha3 512 hash.
	 */
	hash512: 64,

	/**
	 * @property {numeric} Size of VRF proof properties.
	 */
	vrfProof: {
		gamma: 32,
		verificationHash: 16,
		scalar: 32
	},

	/**
	 * @property {numeric} Size of a voting key.
	 */
	votingKey: 48
};

module.exports = sizes;
