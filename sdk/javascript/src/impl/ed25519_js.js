// this file contains implementation details and is not intended to be used directly

import tweetnacl from './external/tweetnacl-nacl-fast-symbol.js';
import { deepCompare } from '../utils/arrayHelpers.js';
import { keccak_512 } from '@noble/hashes/sha3';
import { sha512 } from '@noble/hashes/sha512';

const getHasher = hashMode => ('Keccak' === hashMode ? keccak_512 : sha512);

const isCanonicalS = encodedS => {
	const reduce = r => {
		const x = new Float64Array(64);
		let i;
		for (i = 0; 64 > i; i++)
			x[i] = r[i];

		for (i = 0; 64 > i; i++)
			r[i] = 0;

		tweetnacl.lowlevel.modL(r, x);
	};

	// require canonical signature
	const reducedEncodedS = new Uint8Array([...encodedS, ...new Uint8Array(32)]);
	reduce(reducedEncodedS);
	return 0 === deepCompare(encodedS, reducedEncodedS.subarray(0, 32));
};

const ed25519 = {
	keyPairFromSeed: (hashMode, seed) => tweetnacl.sign.keyPair.fromSeed(seed, getHasher(hashMode)),
	sign: (hashMode, message, keyPair) => tweetnacl.sign.detached(message, keyPair.secretKey, getHasher(hashMode)),
	verify: (hashMode, message, signature, publicKey) => tweetnacl.sign.detached.verify(message, signature, publicKey, getHasher(hashMode))
		&& isCanonicalS(signature.subarray(32, 64))
};

export default ed25519;
