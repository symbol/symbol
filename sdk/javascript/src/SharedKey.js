// this file contains implementation details and is not intended to be used directly

import {
	/* eslint-disable no-unused-vars */
	PublicKey,
	/* eslint-enable no-unused-vars */
	SharedKey256
} from './CryptoTypes.js';
import { hkdf } from '@noble/hashes/hkdf';
import { sha256 } from '@noble/hashes/sha256';
import tweetnacl from 'tweetnacl';

// order matches order of exported methods
const tweetnacl_lowlevel = (/** @type {any} */ (tweetnacl)).lowlevel;
const {
	crypto_verify_32, gf, pack25519, unpack25519, pow2523, set25519
} = tweetnacl_lowlevel;

// region curve operations - unfortunatelly tweetnacl.lowlevel does not expose those functions, so needed to copy them here

const neq25519 = (a, b) => {
	const c = new Uint8Array(32);
	const d = new Uint8Array(32);
	pack25519(c, a);
	pack25519(d, b);
	return crypto_verify_32(c, 0, d, 0);
};

const par25519 = a => {
	const d = new Uint8Array(32);
	pack25519(d, a);
	return d[0] & 1;
};

const inv25519 = (o, i) => {
	const { M, S } = tweetnacl_lowlevel;

	const c = gf();
	for (let a = 0; 16 > a; a++)
		c[a] = i[a];

	for (let a = 253; 0 <= a; a--) {
		S(c, c);
		if (2 !== a && 4 !== a)
			M(c, c, i);
	}
	for (let a = 0; 16 > a; a++)
		o[a] = c[a];
};

const pack = (r, p) => {
	const { M } = tweetnacl_lowlevel;
	const tx = gf();
	const ty = gf();
	const zi = gf();

	inv25519(zi, p[2]);
	M(tx, p[0], zi);
	M(ty, p[1], zi);
	pack25519(r, ty);

	r[31] ^= par25519(tx) << 7;
};

const unpackNeg = (r, p) => {
	const {
		D, M, A, S, Z
	} = tweetnacl_lowlevel;

	const gf0 = gf();
	const gf1 = gf([1]);
	const I = gf([
		0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43, 0xd7a7, 0x3dfb, 0x0099, 0x2b4d, 0xdf0b, 0x4fc1, 0x2480, 0x2b83
	]);

	const t = gf();
	const chk = gf();
	const num = gf();
	const den = gf();
	const den2 = gf();
	const den4 = gf();
	const den6 = gf();

	set25519(r[2], gf1);
	unpack25519(r[1], p);
	S(num, r[1]);
	M(den, num, D);
	Z(num, num, r[2]);
	A(den, r[2], den);

	S(den2, den);
	S(den4, den2);
	M(den6, den4, den2);
	M(t, den6, num);
	M(t, t, den);

	pow2523(t, t);
	M(t, t, num);
	M(t, t, den);
	M(t, t, den);
	M(r[0], t, den);

	S(chk, r[0]);
	M(chk, chk, den);
	if (neq25519(chk, num))
		M(r[0], r[0], I);

	S(chk, r[0]);
	M(chk, chk, den);
	if (neq25519(chk, num))
		return -1;

	if (par25519(r[0]) === (p[31] >> 7))
		Z(r[0], gf0, r[0]);

	M(r[3], r[0], r[1]);
	return 0;
};

// endregion

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
 * @param {function} cryptoHash Hash function to use.
 * @returns {function(Uint8Array, PublicKey): Uint8Array} Creates a shared secret from a raw private key and public key.
 */
const deriveSharedSecretFactory = cryptoHash => (privateKeyBytes, otherPublicKey) => {
	const { scalarmult, Z } = tweetnacl_lowlevel;
	const point = [gf(), gf(), gf(), gf()];

	if (!isCanonicalKey(otherPublicKey) || 0 !== unpackNeg(point, otherPublicKey.bytes) || !isInMainSubgroup(point))
		throw Error('invalid point');

	// negate point == negate X coordinate and 't'
	Z(point[0], gf(), point[0]);
	Z(point[3], gf(), point[3]);

	const scalar = new Uint8Array(64);

	cryptoHash(scalar, privateKeyBytes, 32);
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
 * @param {function} cryptoHash Hash function to use.
 * @returns {function(Uint8Array, PublicKey): SharedKey256} Creates a shared key from a raw private key and public key.
 */
const deriveSharedKeyFactory = (info, cryptoHash) => {
	const deriveSharedSecret = deriveSharedSecretFactory(cryptoHash);
	return (privateKeyBytes, otherPublicKey) => {
		const sharedSecret = deriveSharedSecret(privateKeyBytes, otherPublicKey);
		return new SharedKey256(hkdf(sha256, sharedSecret, undefined, info, 32));
	};
};

export {
	deriveSharedSecretFactory,
	deriveSharedKeyFactory
};
