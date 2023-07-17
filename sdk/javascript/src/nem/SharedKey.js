/* eslint-disable no-unused-vars */
import { KeyPair } from './KeyPair.js';
/* eslint-enable no-unused-vars */
import {
	/* eslint-disable no-unused-vars */
	PublicKey,
	/* eslint-enable no-unused-vars */
	SharedKey256
} from '../CryptoTypes.js';
import { deriveSharedKeyFactory, deriveSharedSecretFactory } from '../SharedKey.js';
import { keccak_256, keccak_512 } from '@noble/hashes/sha3';

const crypto_hash = (out, m, n) => {
	const hashBuilder = keccak_512.create();
	hashBuilder.update(m.subarray(0, n));
	const hash = hashBuilder.digest();

	for (let i = 0; i < out.length; ++i)
		out[i] = hash[i];

	return 0;
};
const deriveSharedSecretImpl = deriveSharedSecretFactory(crypto_hash);
const deriveSharedKeyImpl = deriveSharedKeyFactory('nem-nis1', crypto_hash);

/**
 * Derives shared key from key pair and other party's public key.
 * @param {KeyPair} keyPair Key pair.
 * @param {PublicKey} otherPublicKey Other party's public key.
 * @returns {SharedKey256} Shared encryption key.
 */
const deriveSharedKey = (keyPair, otherPublicKey) => {
	const reversedPrivateKeyBytes = new Uint8Array([...keyPair.privateKey.bytes]);
	reversedPrivateKeyBytes.reverse();

	return deriveSharedKeyImpl(reversedPrivateKeyBytes, otherPublicKey);
};

/**
 * Derives shared key from key pair, other party's public key and salt
 * @deprecated This is _old_ method of deriving shared key and should not be used in new applications.
 * @param {KeyPair} keyPair Key pair.
 * @param {PublicKey} otherPublicKey Other party's public key.
 * @param {Uint8Array} salt Random salt. Should be unique per every use.
 * @returns {SharedKey256} Shared encryption key.
 */
const deriveSharedKeyDeprecated = (keyPair, otherPublicKey, salt) => {
	if (SharedKey256.SIZE !== salt.length)
		throw Error('invalid salt');

	const reversedPrivateKeyBytes = new Uint8Array([...keyPair.privateKey.bytes]);
	reversedPrivateKeyBytes.reverse();

	const sharedSecret = deriveSharedSecretImpl(reversedPrivateKeyBytes, otherPublicKey);
	const sharedKeyBytes = new Uint8Array(SharedKey256.SIZE);
	for (let i = 0; SharedKey256.SIZE > i; ++i)
		sharedKeyBytes[i] = sharedSecret[i] ^ salt[i];

	return new SharedKey256(keccak_256(sharedKeyBytes));
};

export {
	deriveSharedKey,
	deriveSharedKeyDeprecated
};
