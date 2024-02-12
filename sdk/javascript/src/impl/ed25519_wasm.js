// this file contains implementation details and is not intended to be used directly

/* eslint-disable import/no-extraneous-dependencies */
import {
	HashMode, crypto_private_sign, crypto_private_verify, crypto_sign_keypair
} from 'symbol-crypto-wasm-node';
/* eslint-enable import/no-extraneous-dependencies */

const CRYPTO_SIGN_BYTES = 64;
const CRYPTO_SIGN_PUBLICKEYBYTES = 32;

const ed25519 = {
	keyPairFromSeed: (hashMode, seed) => {
		const publicKey = new Uint8Array(CRYPTO_SIGN_PUBLICKEYBYTES);
		crypto_sign_keypair(HashMode[hashMode], seed, publicKey);
		return { publicKey, privateKey: seed };
	},
	sign: (hashMode, message, keyPair) => {
		const signature = new Uint8Array(CRYPTO_SIGN_BYTES);
		crypto_private_sign(HashMode[hashMode], keyPair.privateKey, message, signature);
		return signature;
	},
	verify: (hashMode, message, signature, publicKey) => crypto_private_verify(HashMode[hashMode], publicKey, message, signature)
};

export default ed25519;
