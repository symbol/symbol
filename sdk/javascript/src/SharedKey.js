// this file contains implementation details and is not intended to be used directly

import {
	/* eslint-disable no-unused-vars */
	PublicKey,
	/* eslint-enable no-unused-vars */
	SharedKey256
} from './CryptoTypes.js';
import tweetnacl from './impl/external/tweetnacl-nacl-fast-symbol.js';
import { hkdf } from '@noble/hashes/hkdf';
import { sha256 } from '@noble/hashes/sha256';

// order matches order of exported methods
const tweetnacl_lowlevel = (/** @type {any} */ (tweetnacl)).lowlevel;
const {
	gf, neq25519, pack, unpackneg
} = tweetnacl_lowlevel;

// publicKey is canonical if the y coordinate is smaller than 2^255 - 19
// note: this version is based on server version and should be constant-time
// note 2: don't touch it, you'll break it
const isCanonicalKey = publicKey => {
	const buffer = publicKey.bytes;
	let a = (buffer[31] & 0x7F) ^ 0x7F;
	for (let i = 30; 0 < i; --i)
		a |= buffer[i] ^ 0xFF;

	a = (a - 1) >>> 8;

	const b = (0xED - 1 - buffer[0]) >>> 8;
	return 0 !== 1 - (a & b & 1);
};

const isInMainSubgroup = point => {
	const { scalarmult, L } = tweetnacl_lowlevel;
	const result = [gf(), gf(), gf(), gf()];
	// multiply by group order
	scalarmult(result, point, L);

	// check if result is neutral element
	const gf0 = gf();
	const areEqual = neq25519(result[1], result[2]);
	const isZero = neq25519(gf0, result[0]);

	// yes, this is supposed to be  bit OR
	return 0 === (areEqual | isZero);
};

/**
 * Creates a shared secret factory given a hash function.
 * @param {object} hasher Hash object to use.
 * @returns {function(Uint8Array, PublicKey): Uint8Array} Creates a shared secret from a raw private key and public key.
 */
const deriveSharedSecretFactory = hasher => (privateKeyBytes, otherPublicKey) => {
	const { scalarmult, Z } = tweetnacl_lowlevel;
	const point = [gf(), gf(), gf(), gf()];

	if (!isCanonicalKey(otherPublicKey) || 0 !== unpackneg(point, otherPublicKey.bytes) || !isInMainSubgroup(point))
		throw Error('invalid point');

	// negate point == negate X coordinate and 't'
	Z(point[0], gf(), point[0]);
	Z(point[3], gf(), point[3]);

	const scalar = new Uint8Array(64);

	tweetnacl.lowlevel.crypto_hash(scalar, privateKeyBytes, 32, hasher);
	scalar[0] &= 248;
	scalar[31] &= 127;
	scalar[31] |= 64;

	const result = [gf(), gf(), gf(), gf()];
	scalarmult(result, point, scalar);

	const sharedSecret = new Uint8Array(32);
	pack(sharedSecret, result);
	return sharedSecret;
};

/**
 * Creates a shared key factory given a tag and a hash function.
 * @param {string} info Tag used in HKDF algorithm.
 * @param {object} hasher Hash object to use.
 * @returns {function(Uint8Array, PublicKey): SharedKey256} Creates a shared key from a raw private key and public key.
 */
const deriveSharedKeyFactory = (info, hasher) => {
	const deriveSharedSecret = deriveSharedSecretFactory(hasher);
	return (privateKeyBytes, otherPublicKey) => {
		const sharedSecret = deriveSharedSecret(privateKeyBytes, otherPublicKey);
		return new SharedKey256(hkdf(sha256, sharedSecret, undefined, info, 32));
	};
};

export {
	deriveSharedSecretFactory,
	deriveSharedKeyFactory
};
