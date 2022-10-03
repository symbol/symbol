import {
	Hash256, PrivateKey, PublicKey, SharedKey256, Signature
} from '../src/CryptoTypes.js';
import { expect } from 'chai';
import crypto from 'crypto';

describe('CryptoTypes', () => {
	// region test utils

	const assertCanCreateByteArrayWithCorrectNumberOfBytes = (ByteArrayClass, size) => {
		// Arrange:
		const rawBytes = crypto.randomBytes(size);

		// Act:
		const byteArray = new ByteArrayClass(rawBytes);

		// Assert:
		expect(byteArray.bytes).to.deep.equal(rawBytes);
	};

	const assertCannotCreateByteArrayWithIncorrectNumberOfBytes = (ByteArrayClass, requiredSize) => {
		[0, requiredSize - 1, requiredSize + 1].forEach(size => {
			expect(() => {
				new ByteArrayClass(size, crypto.randomBytes(size)); // eslint-disable-line no-new
			}).to.throw('bytes was size');
		});
	};

	const assertCanCreateZeroedByteArrayWithCorrectNumberOfBytes = (ByteArrayClass, size) => {
		// Act:
		const byteArray = ByteArrayClass.zero();

		// Assert:
		expect(byteArray.bytes).to.deep.equal(new Uint8Array(size));
	};

	// endregion

	describe('Hash256', () => {
		it('can create hash256 with correct number of bytes', () => {
			assertCanCreateByteArrayWithCorrectNumberOfBytes(Hash256, 32);
		});

		it('cannot create hash256 with incorrect number of bytes', () => {
			assertCannotCreateByteArrayWithIncorrectNumberOfBytes(Hash256, 32);
		});

		it('can create zeroed hash256 with correct number of bytes', () => {
			assertCanCreateZeroedByteArrayWithCorrectNumberOfBytes(Hash256, 32);
		});
	});

	describe('PrivateKey', () => {
		it('can create private key with correct number of bytes', () => {
			assertCanCreateByteArrayWithCorrectNumberOfBytes(PrivateKey, 32);
		});

		it('cannot create private key with incorrect number of bytes', () => {
			assertCannotCreateByteArrayWithIncorrectNumberOfBytes(PrivateKey, 32);
		});

		it('can create random private key', () => {
			// Act:
			const privateKey1 = PrivateKey.random();
			const privateKey2 = PrivateKey.random();

			// Assert:
			expect(privateKey1).to.not.deep.equal(privateKey2);
		});
	});

	describe('PublicKey', () => {
		it('can create public key with correct number of bytes', () => {
			assertCanCreateByteArrayWithCorrectNumberOfBytes(PublicKey, 32);
		});

		it('cannot create public key with incorrect number of bytes', () => {
			assertCannotCreateByteArrayWithIncorrectNumberOfBytes(PublicKey, 32);
		});

		it('can create public key from existing public key', () => {
			// Arrange:
			const rawBytes = crypto.randomBytes(PublicKey.SIZE);

			// Act:
			const byteArray = new PublicKey(new PublicKey(rawBytes));

			// Assert:
			expect(byteArray.bytes).to.deep.equal(rawBytes);
		});
	});

	describe('SharedKey256', () => {
		it('can create shared key with correct number of bytes', () => {
			assertCanCreateByteArrayWithCorrectNumberOfBytes(SharedKey256, 32);
		});

		it('cannot create shared key with incorrect number of bytes', () => {
			assertCannotCreateByteArrayWithIncorrectNumberOfBytes(SharedKey256, 32);
		});
	});

	describe('Signature', () => {
		it('can create signature with correct number of bytes', () => {
			assertCanCreateByteArrayWithCorrectNumberOfBytes(Signature, 64);
		});

		it('cannot create signature with incorrect number of bytes', () => {
			assertCannotCreateByteArrayWithIncorrectNumberOfBytes(Signature, 64);
		});

		it('can create zeroed signature with correct number of bytes', () => {
			assertCanCreateZeroedByteArrayWithCorrectNumberOfBytes(Signature, 64);
		});
	});
});
