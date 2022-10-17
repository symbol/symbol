import { PublicKey, Signature } from '../../src/CryptoTypes.js';
import { expect } from 'chai';
import crypto from 'crypto';

export const runBasicTransactionFactoryTests = ( // eslint-disable-line import/prefer-default-export
	testDescriptor,
	includeAttachSignatureTests = true
) => {
	const TEST_SIGNER_PUBLIC_KEY = new PublicKey(crypto.randomBytes(PublicKey.SIZE));
	const transactionTypeName = testDescriptor.transactionTypeName || 'transfer_transaction_v1';

	// region create

	it('can create known transaction from descriptor', () => {
		// Arrange:
		const factory = testDescriptor.createFactory();

		// Act:
		const transaction = testDescriptor.createTransaction(factory)({
			type: transactionTypeName,
			signerPublicKey: TEST_SIGNER_PUBLIC_KEY
		});

		// Assert:
		testDescriptor.assertTransaction(transaction);
		expect(transaction.signerPublicKey).to.deep.equal(TEST_SIGNER_PUBLIC_KEY);
	});

	it('cannot create unknown transaction from descriptor', () => {
		// Arrange:
		const factory = testDescriptor.createFactory();

		// Act + Assert:
		expect(() => {
			testDescriptor.createTransaction(factory)({
				type: `x${transactionTypeName}`,
				signerPublicKey: TEST_SIGNER_PUBLIC_KEY
			});
		}).to.throw(`unknown ${testDescriptor.name} type`);
	});

	// endregion

	if (includeAttachSignatureTests) {
		// region attachSignature

		it('can attach signature to transaction', () => {
			// Arrange:
			const factory = testDescriptor.createFactory();
			const transaction = testDescriptor.createTransaction(factory)({
				type: transactionTypeName,
				signerPublicKey: TEST_SIGNER_PUBLIC_KEY
			});
			const signature = new Signature(crypto.randomBytes(Signature.SIZE));

			// Sanity:
			expect(signature).to.not.deep.equal(Signature.zero());

			// Act:
			const signedTransactionPayload = factory.constructor.attachSignature(transaction, signature);

			// Assert:
			testDescriptor.assertTransaction(transaction);
			expect(transaction.signerPublicKey).to.deep.equal(TEST_SIGNER_PUBLIC_KEY);
			expect(transaction.signature).to.deep.equal(signature);

			testDescriptor.assertSignature(transaction, signature, signedTransactionPayload);
		});

		// endregion
	}
};
