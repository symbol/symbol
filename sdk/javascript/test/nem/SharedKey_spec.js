import { PrivateKey } from '../../src/CryptoTypes.js';
import NemFacade from '../../src/facade/NemFacade.js';
import { KeyPair } from '../../src/nem/KeyPair.js';
import { deriveSharedKeyDeprecated } from '../../src/nem/SharedKey.js'; // eslint-disable-line import/no-deprecated
import runBasicSharedKeyTests from '../test/sharedKeyTests.js';
import { expect } from 'chai';

describe('SharedKey (NEM)', () => {
	runBasicSharedKeyTests({
		KeyPair,
		deriveSharedKey: NemFacade.deriveSharedKey
	});
});

describe('SharedKey (NEM) (deprecated)', () => {
	const deterministicSalt = new TextEncoder().encode('1234567890ABCDEF1234567890ABCDEF');
	runBasicSharedKeyTests({
		KeyPair,
		// eslint-disable-next-line import/no-deprecated
		deriveSharedKey: (keyPair, publicKey) => deriveSharedKeyDeprecated(keyPair, publicKey, deterministicSalt)
	});

	it('salt with invalid length is rejected', () => {
		// Arrange:
		const keyPair = new KeyPair(PrivateKey.random());
		const otherPublicKey = new KeyPair(PrivateKey.random()).publicKey;

		// Act:
		// eslint-disable-next-line import/no-deprecated
		expect(() => deriveSharedKeyDeprecated(keyPair, otherPublicKey, new Uint8Array(31))).to.throw('invalid salt');
		// eslint-disable-next-line import/no-deprecated
		expect(() => deriveSharedKeyDeprecated(keyPair, otherPublicKey, deterministicSalt.subarray(0, 31))).to.throw('invalid salt');
		// eslint-disable-next-line import/no-deprecated
		expect(() => deriveSharedKeyDeprecated(keyPair, otherPublicKey, new Uint8Array(33))).to.throw('invalid salt');
	});
});
