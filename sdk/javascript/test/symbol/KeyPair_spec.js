const { PrivateKey, PublicKey } = require('../../src/CryptoTypes');
const { KeyPair, Verifier } = require('../../src/symbol/KeyPair');
const { runBasicKeyPairTests } = require('../test/keyPairTests');

describe('KeyPair (Symbol)', () => {
	runBasicKeyPairTests({
		KeyPair,
		Verifier,
		deterministicPrivateKey: new PrivateKey('E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E'),
		expectedPublicKey: new PublicKey('E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD')
	});
});
