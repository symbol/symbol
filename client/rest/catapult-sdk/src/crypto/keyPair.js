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

/** @module crypto/keyPair */
const arrayUtils = require('../utils/arrayUtils');
const convert = require('../utils/convert');
const nacl = require('../external/nacl_catapult');
const sha3Hasher = require('./sha3Hasher');

const Key_Size = 32;
const Signature_Size = 64;
const Half_Signature_Size = Signature_Size / 2;
const Hash_Size = 64;
const Half_Hash_Size = Hash_Size / 2;

// custom catapult hash functions
const catapult = {};
catapult.hash = {
	func: sha3Hasher.func,
	createHasher: sha3Hasher.createHasher
};

// custom catapult crypto functions
catapult.crypto = (() => {
	const clamp = d => {
		d[0] &= 248;
		d[31] &= 127;
		d[31] |= 64;
	};

	const prepareForScalarMult = (sk, hashfunc) => {
		const d = new Uint8Array(Hash_Size);
		hashfunc(d, sk);
		clamp(d);
		return d;
	};

	const encodedSChecker = (() => {
		const Is_Reduced = 1;
		const Is_Zero = 2;

		const validateEncodedSPart = s => {
			if (arrayUtils.isZero(s))
				return Is_Zero | Is_Reduced;

			const copy = new Uint8Array(Signature_Size);
			arrayUtils.copy(copy, s, Half_Signature_Size);

			nacl.catapult.reduce(copy);
			return arrayUtils.deepEqual(s, copy, Half_Signature_Size) ? Is_Reduced : 0;
		};

		return {
			isCanonical: s => Is_Reduced === validateEncodedSPart(s),

			requireValid: s => {
				if (0 === (validateEncodedSPart(s) & Is_Reduced))
					throw Error('S part of signature invalid');
			}
		};
	})();

	return {
		extractPublicKey: (sk, hashfunc) => {
			const c = nacl.catapult;
			const d = prepareForScalarMult(sk, hashfunc);

			const p = [c.gf(), c.gf(), c.gf(), c.gf()];
			const pk = new Uint8Array(Key_Size);
			c.scalarbase(p, d);
			c.pack(pk, p);
			return pk;
		},

		sign: (m, pk, sk, hasher) => {
			const c = nacl.catapult;

			const d = new Uint8Array(Hash_Size);
			hasher.reset();
			hasher.update(sk);
			hasher.finalize(d);
			clamp(d);

			const r = new Uint8Array(Hash_Size);
			hasher.reset();
			hasher.update(d.subarray(Half_Hash_Size));
			hasher.update(m);
			hasher.finalize(r);

			const p = [c.gf(), c.gf(), c.gf(), c.gf()];
			const signature = new Uint8Array(Signature_Size);
			c.reduce(r);
			c.scalarbase(p, r);
			c.pack(signature, p);

			const h = new Uint8Array(Hash_Size);
			hasher.reset();
			hasher.update(signature.subarray(0, Half_Signature_Size));
			hasher.update(pk);
			hasher.update(m);
			hasher.finalize(h);

			c.reduce(h);

			// muladd
			const x = new Float64Array(Hash_Size);
			arrayUtils.copy(x, r, Half_Hash_Size);

			for (let i = 0; i < Half_Hash_Size; ++i) {
				for (let j = 0; j < Half_Hash_Size; ++j)
					x[i + j] += h[i] * d[j];
			}

			c.modL(signature.subarray(Half_Signature_Size), x);
			encodedSChecker.requireValid(signature.subarray(Half_Signature_Size));
			return signature;
		},

		verify: (pk, m, signature, hasher) => {
			// reject non canonical signature
			if (!encodedSChecker.isCanonical(signature.subarray(Half_Signature_Size)))
				return false;

			// reject weak (zero) public key
			if (arrayUtils.isZero(pk))
				return false;

			const c = nacl.catapult;
			const p = [c.gf(), c.gf(), c.gf(), c.gf()];
			const q = [c.gf(), c.gf(), c.gf(), c.gf()];

			if (c.unpackneg(q, pk))
				return false;

			const h = new Uint8Array(Hash_Size);
			hasher.reset();
			hasher.update(signature.subarray(0, Half_Signature_Size));
			hasher.update(pk);
			hasher.update(m);
			hasher.finalize(h);

			c.reduce(h);
			c.scalarmult(p, q, h);

			const t = new Uint8Array(Signature_Size);
			c.scalarbase(q, signature.subarray(Half_Signature_Size));
			c.add(p, q);
			c.pack(t, p);

			return 0 === c.crypto_verify_32(signature, 0, t, 0);
		},

		deriveSharedKey: (salt, sk, pk, hashfunc) => {
			const c = nacl.catapult;
			const d = prepareForScalarMult(sk, hashfunc);

			// sharedKey = pack(p = d (derived from sk) * q (derived from pk))
			const q = [c.gf(), c.gf(), c.gf(), c.gf()];
			const p = [c.gf(), c.gf(), c.gf(), c.gf()];
			const sharedKey = new Uint8Array(Key_Size);
			c.unpackneg(q, pk);
			c.scalarmult(p, q, d);
			c.pack(sharedKey, p);

			// salt the shared key
			for (let i = 0; i < Key_Size; ++i)
				sharedKey[i] ^= salt[i];

			// return the hash of the result
			const sharedKeyHash = new Uint8Array(Key_Size);
			hashfunc(sharedKeyHash, sharedKey, Key_Size);
			return sharedKeyHash;
		}
	};
})();

// region exported functions

/**
 * A catapult public key.
 * @typedef {Uint8Array} PublicKey
 */

/**
 * A catapult key pair composed of a public and private key.
 * @typedef {object} KeyPair
 * @property {module:crypto/keyPair~PublicKey} publicKey The public key.
 * @property {Uint8Array} privateKey The private key.
 */

const keyPairModule = {
	/**
 	 * Creates a key pair from a private key string.
	 * @param {string} privateKeyString A hex encoded private key string.
	 * @returns {module:crypto/keyPair~KeyPair} The key pair.
	 */
	createKeyPairFromPrivateKeyString: privateKeyString => {
		const privateKey = convert.hexToUint8(privateKeyString);
		if (Key_Size !== privateKey.length)
			throw Error(`private key has unexpected size: ${privateKey.length}`);

		const publicKey = catapult.crypto.extractPublicKey(privateKey, catapult.hash.func);
		return { privateKey, publicKey };
	},

	/**
	 * Signs a data buffer with a key pair.
	 * @param {module:crypto/keyPair~KeyPair} keyPair The key pair to use for signing.
	 * @param {Uint8Array} data The data to sign.
	 * @returns {Uint8Array} The signature.
	 */
	sign: (keyPair, data) => catapult.crypto.sign(data, keyPair.publicKey, keyPair.privateKey, catapult.hash.createHasher()),

	/**
	 * Verifies a signature.
	 * @param {module:crypto/keyPair~PublicKey} publicKey The public key to use for verification.
	 * @param {Uint8Array} data The data to verify.
	 * @param {Uint8Array} signature The signature to verify.
	 * @returns {boolean} true if the signature is verifiable, false otherwise.
	 */
	verify: (publicKey, data, signature) => catapult.crypto.verify(publicKey, data, signature, catapult.hash.createHasher()),

	/**
	 * Creates a shared key given a key pair and an arbitrary public key.
	 * The shared key can be used for encrypted message passing between the two.
	 * @param {module:crypto/keyPair~KeyPair} keyPair The key pair for which to create the shared key.
	 * @param {module:crypto/keyPair~PublicKey} publicKey The public key for which to create the shared key.
	 * @param {Uint8Array} salt A salt that should be applied to the shared key.
	 * @returns {Uint8Array} The shared key.
	 */
	deriveSharedKey: (keyPair, publicKey, salt) => {
		if (Key_Size !== salt.length)
			throw Error(`salt has unexpected size: ${salt.length}`);

		return catapult.crypto.deriveSharedKey(salt, keyPair.privateKey, publicKey, catapult.hash.func);
	}
};

// endregion

module.exports = keyPairModule;
