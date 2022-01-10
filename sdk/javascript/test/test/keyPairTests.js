const { PrivateKey, PublicKey, Signature } = require('../../src/CryptoTypes');
const { expect } = require('chai');
const crypto = require('crypto');

const runBasicKeyPairTests = testDescriptor => {
	it('can create key pair from private key', () => {
		// Arrange:
		const publicKey = testDescriptor.expectedPublicKey;
		const privateKey = testDescriptor.deterministicPrivateKey;

		// Act:
		const keyPair = new testDescriptor.KeyPair(privateKey);

		// Assert:
		expect(keyPair.publicKey).to.deep.equal(publicKey);
		expect(keyPair.privateKey).to.deep.equal(privateKey);
	});

	// region sign

	it('sign fills signature', () => {
		// Arrange:
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const message = new Uint8Array(crypto.randomBytes(21));

		// Act:
		const signature = keyPair.sign(message);

		// Assert:
		expect(signature).to.not.deep.equal(Signature.zero());
	});

	it('signatures generated for same data by same key pairs are equal', () => {
		// Arrange:
		const privateKey = PrivateKey.random();
		const keyPair1 = new testDescriptor.KeyPair(privateKey);
		const keyPair2 = new testDescriptor.KeyPair(privateKey);
		const message = new Uint8Array(crypto.randomBytes(21));

		// Act:
		const signature1 = keyPair1.sign(message);
		const signature2 = keyPair2.sign(message);

		// Assert:
		expect(signature2).to.deep.equal(signature1);
	});

	it('signatures generated for same data by different key pairs are different', () => {
		// Arrange:
		const keyPair1 = new testDescriptor.KeyPair(PrivateKey.random());
		const keyPair2 = new testDescriptor.KeyPair(PrivateKey.random());
		const message = new Uint8Array(crypto.randomBytes(21));

		// Act:
		const signature1 = keyPair1.sign(message);
		const signature2 = keyPair2.sign(message);

		// Assert:
		expect(signature2).to.not.deep.equal(signature1);
	});

	// endregion

	// region verify

	it('can verify signature', () => {
		// Arrange:
		const message = new Uint8Array(crypto.randomBytes(21));
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const signature = keyPair.sign(message);

		// Act:
		const isVerified = new testDescriptor.Verifier(keyPair.publicKey).verify(message, signature);

		// Assert:
		expect(isVerified).to.equal(true);
	});

	it('cannot verify signature with different key pair', () => {
		// Arrange:
		const message = new Uint8Array(crypto.randomBytes(21));
		const signature = new testDescriptor.KeyPair(PrivateKey.random()).sign(message);

		// Act:
		const isVerified = new testDescriptor.Verifier(new PublicKey(crypto.randomBytes(PublicKey.SIZE))).verify(message, signature);

		// Assert:
		expect(isVerified).to.equal(false);
	});

	const mutateBytes = (bytes, position) => new Uint8Array([
		...bytes.subarray(0, position),
		bytes[position] ^ 0xFF,
		...bytes.subarray(position + 1)
	]);

	it('cannot verify signature when message is modified', () => {
		// Arrange:
		const message = new Uint8Array(crypto.randomBytes(21));
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const signature = keyPair.sign(message);

		const verifier = new testDescriptor.Verifier(keyPair.publicKey);
		for (let i = 0; i < message.length; ++i) {
			const modifiedMessage = mutateBytes(message, i);

			// Act:
			const isVerified = verifier.verify(modifiedMessage, signature);

			// Assert:
			expect(isVerified).to.equal(false);
		}
	});

	it('cannot verify signature when signature is modified', () => {
		// Arrange:
		const message = new Uint8Array(crypto.randomBytes(21));
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const signature = keyPair.sign(message);

		const verifier = new testDescriptor.Verifier(keyPair.publicKey);
		for (let i = 0; i < message.length; ++i) {
			const modifiedSignature = new Signature(mutateBytes(signature.bytes, i));

			// Act:
			const isVerified = verifier.verify(message, modifiedSignature);

			// Assert:
			expect(isVerified, `modification at index ${i}`).to.equal(false);
		}
	});

	// endregion
};

module.exports = { runBasicKeyPairTests };
