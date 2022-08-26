const {
	crypto_sign_keypair, crypto_private_sign, crypto_private_verify, HashMode
} = require('../../wasm/pkg/symbol_crypto_wasm');

const CRYPTO_SIGN_BYTES = 64;
const CRYPTO_SIGN_PUBLICKEYBYTES = 32;
const CRYPTO_SIGN_SECRETKEYBYTES = 64;

const ed25519 = {
	keyPairFromSeed: (hashMode, seed) => {
		const publicKey = new Uint8Array(CRYPTO_SIGN_PUBLICKEYBYTES);
		crypto_sign_keypair(HashMode[hashMode], seed, publicKey);
		return { publicKey, privateKey: seed };
	},
	sign: (hashMode, message, privateKey) => {
		const signature = new Uint8Array(CRYPTO_SIGN_BYTES);
		crypto_private_sign(HashMode[hashMode], privateKey, message, signature);
		return signature;
	},
	verify: (hashMode, message, signature, publicKey) => crypto_private_verify(HashMode[hashMode], publicKey, message, signature)
};

module.exports = ed25519;
