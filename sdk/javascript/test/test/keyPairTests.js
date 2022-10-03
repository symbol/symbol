import { PrivateKey, PublicKey, Signature } from '../../src/CryptoTypes.js';
import { hexToUint8 } from '../../src/utils/converter.js';
import { expect } from 'chai';
import crypto from 'crypto';

export const runBasicKeyPairTests = testDescriptor => { // eslint-disable-line import/prefer-default-export
	// region create

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

	// endregion

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

	it('cannot create verifier around zero public key', () => {
		// Arrange:
		const zeroPublicKey = new PublicKey(new Uint8Array(PublicKey.SIZE));

		// Act + Assert:
		expect(() => new testDescriptor.Verifier(zeroPublicKey)).to.throw('public key cannot be zero');
	});

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

	it('cannot verify signature with zero s', () => {
		// Arrange:
		const message = new Uint8Array(crypto.randomBytes(21));
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const signature = keyPair.sign(message);
		const signatureZeroS = new Signature(new Uint8Array([...signature.bytes.subarray(0, 32), ...new Uint8Array(32)]));

		const verifier = new testDescriptor.Verifier(keyPair.publicKey);

		// Act:
		const isVerified = verifier.verify(message, signature);
		const isVerifiedZeroS = verifier.verify(message, signatureZeroS);

		// Assert:
		expect(isVerified).to.equal(true);
		expect(isVerifiedZeroS).to.equal(false);
	});

	const scalarAddGroupOrder = scalar => {
		// 2^252 + 27742317777372353535851937790883648493, little endian
		const groupOrder = hexToUint8('EDD3F55C1A631258D69CF7A2DEF9DE1400000000000000000000000000000010');
		let remainder = 0;

		groupOrder.forEach((groupOrderByte, i) => {
			const byteSum = scalar[i] + groupOrderByte;
			scalar[i] = (byteSum + remainder) & 0xFF;
			remainder = (byteSum >>> 8) & 0xFF;
		});

		return scalar;
	};

	it('cannot verify non canonical signature', () => {
		// Arrange:
		// the value 30 in the payload ensures that the encodedS part of the signature is < 2 ^ 253 after adding the group order
		const message = hexToUint8('0102030405060708091D');
		const keyPair = new testDescriptor.KeyPair(PrivateKey.random());
		const canonicalSignature = keyPair.sign(message);

		const nonCanonicalSignature = new Signature(new Uint8Array([
			...canonicalSignature.bytes.subarray(0, 32),
			...scalarAddGroupOrder(canonicalSignature.bytes.slice(32, 64))
		]));

		const verifier = new testDescriptor.Verifier(keyPair.publicKey);

		// Act:
		const isVerifiedCanonical = verifier.verify(message, canonicalSignature);
		const isVerifiedNonCanonical = verifier.verify(message, nonCanonicalSignature);

		// Assert:
		expect(isVerifiedCanonical).to.equal(true);
		expect(isVerifiedNonCanonical).to.equal(false);
	});

	// endregion
};
