const { PrivateKey, PublicKey } = require('../../src/CryptoTypes');
const { expect } = require('chai');

const runBasicSharedKeyTests = testDescriptor => {
	// region mutual shared

	const assertDerivedSharedResult = (mutate, assertion) => {
		// Arrange:
		const privateKey1 = PrivateKey.random();
		const otherPublicKey1 = new testDescriptor.KeyPair(PrivateKey.random()).publicKey;

		const privateKey2Bytes = new Uint8Array(privateKey1.bytes);
		const otherPublicKey2Bytes = new Uint8Array(otherPublicKey1.bytes);

		mutate(privateKey2Bytes, otherPublicKey2Bytes);

		const keyPair1 = new testDescriptor.KeyPair(privateKey1);
		const keyPair2 = new testDescriptor.KeyPair(new PrivateKey(privateKey2Bytes));

		// Act:
		const sharedKey1 = testDescriptor.deriveSharedKey(keyPair1, otherPublicKey1);
		const sharedKey2 = testDescriptor.deriveSharedKey(keyPair2, new PublicKey(otherPublicKey2Bytes));

		// Assert:
		assertion(sharedKey1, sharedKey2);
	};

	it('shared keys generated with same inputs are equal', () => {
		assertDerivedSharedResult(() => {}, (lhs, rhs) => expect(lhs).to.deep.equal(rhs));
	});

	it('shared keys generated for different private keys are different', () => {
		assertDerivedSharedResult(privateKeyBytes => { privateKeyBytes[0] ^= 0xFF; }, (lhs, rhs) => expect(lhs).to.not.deep.equal(rhs));
	});

	it('shared keys generated for different other public keys are different', () => {
		//  this test needs to be fired multiple times, so that mutated public key will not to be rejected via subgroup check
		for (let i = 1; 256 > i; ++i) {
			try {
				const mutate = (_, otherPublicKey) => { otherPublicKey[0] ^= i; };
				assertDerivedSharedResult(mutate, (lhs, rhs) => expect(lhs).to.not.deep.equal(rhs));
				break;
			} catch (error) {
				// comments for eslint purposes, no need to handle this
			}
		}
	});

	it('mutual shared results are equal', () => {
		// Arrange:
		const keyPair1 = new testDescriptor.KeyPair(PrivateKey.random());
		const keyPair2 = new testDescriptor.KeyPair(PrivateKey.random());

		// Act:
		const sharedKey1 = testDescriptor.deriveSharedKey(keyPair1, keyPair2.publicKey);
		const sharedKey2 = testDescriptor.deriveSharedKey(keyPair2, keyPair1.publicKey);

		// Assert:
		expect(sharedKey1).to.deep.equal(sharedKey2);
	});

	// endregion

	// region invalid public key

	it('public key not on the curve throws', () => {
		// Arrange:
		const keyPair1 = new testDescriptor.KeyPair(PrivateKey.random());
		const keyPair2 = new testDescriptor.KeyPair(PrivateKey.random());

		// this test needs to be fired multiple times, because just setting byte to 1, will likely end up
		// in a point that will still land on the curve
		// note: cannot use expect().to.throw
		let succeeded = false;
		for (let i = 0; 4 > i; ++i) {
			const invalidPublicKeyBytes = new Uint8Array(keyPair2.publicKey.bytes);
			invalidPublicKeyBytes[31] = i;

			// Act + Assert:
			try {
				testDescriptor.deriveSharedKey(keyPair1, new PublicKey(invalidPublicKeyBytes));
			} catch (error) {
				if (error.message.includes('invalid point')) {
					succeeded = true;
					break;
				}
			}
		}

		expect(succeeded).to.equal(true);
	});

	// endregion
};

module.exports = { runBasicSharedKeyTests };
