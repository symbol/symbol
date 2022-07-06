const { PrivateKey } = require('../../src/CryptoTypes');
const { NemFacade } = require('../../src/facade/NemFacade');
const { KeyPair } = require('../../src/nem/KeyPair');
const { deriveSharedKeyDeprecated } = require('../../src/nem/SharedKey');
const { runBasicSharedKeyTests } = require('../test/sharedKeyTests');
const { expect } = require('chai');

describe('SharedKey (NEM)', () => {
	runBasicSharedKeyTests({
		KeyPair,
		deriveSharedKey: NemFacade.deriveSharedKey
	});
});

describe('SharedKey (NEM) (deprecated)', () => {
	const deterministicSalt = new TextEncoder('utf-8').encode('1234567890ABCDEF1234567890ABCDEF');
	runBasicSharedKeyTests({
		KeyPair,
		deriveSharedKey: (keyPair, publicKey) => deriveSharedKeyDeprecated(keyPair, publicKey, deterministicSalt)
	});

	it('salt with invalid length is rejected', () => {
		// Arrange:
		const keyPair = new KeyPair(PrivateKey.random());
		const otherPublicKey = new KeyPair(PrivateKey.random()).publicKey;

		// Act:
		expect(() => deriveSharedKeyDeprecated(keyPair, otherPublicKey, new Uint8Array(31))).to.throw('invalid salt');
		expect(() => deriveSharedKeyDeprecated(keyPair, otherPublicKey, deterministicSalt.subarray(0, 31))).to.throw('invalid salt');
		expect(() => deriveSharedKeyDeprecated(keyPair, otherPublicKey, new Uint8Array(33))).to.throw('invalid salt');
	});
});
