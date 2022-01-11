const { PrivateKey, PublicKey } = require('../../src/CryptoTypes');
const { KeyPair, Verifier } = require('../../src/nem/KeyPair');
const { runBasicKeyPairTests } = require('../test/keyPairTests');

describe('KeyPair (NEM)', () => {
	runBasicKeyPairTests({
		KeyPair,
		Verifier,
		deterministicPrivateKey: new PrivateKey('ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57'), // reversed
		expectedPublicKey: new PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0')
	});
});
