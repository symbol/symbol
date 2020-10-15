/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

const sizes = require('../../src/modelBinary/sizes');
const transactionExtensions = require('../../src/modelBinary/transactionExtensions');
const test = require('../testUtils');
const { expect } = require('chai');

describe('transaction extensions', () => {
	const createMockTransaction = (alpha, beta) => ({
		verifiableEntityHeader_Reserved1: 0,
		signerPublicKey: test.random.bytes(sizes.signerPublicKey),
		signature: test.random.bytes(sizes.signature),
		entityBody_Reserved1: 0,
		alpha,
		beta
	});

	const codec = {
		serialize: (transaction, serializer) => {
			// write header (will be ignored)
			serializer.writeUint32(sizes.transactionHeader + 2 + 4);
			serializer.writeUint32(transaction.verifiableEntityHeader_Reserved1);
			serializer.writeBuffer(transaction.signature);
			serializer.writeBuffer(transaction.signerPublicKey);
			serializer.writeUint32(transaction.entityBody_Reserved1);

			// write data
			serializer.writeUint16(transaction.alpha);
			serializer.writeUint32(transaction.beta);
		}
	};

	describe('hash', () => {
		it('changes if r part of signature changes', () => {
			// Arrange:
			const transaction = createMockTransaction(12, 56);
			const hash = transactionExtensions.hash(codec, transaction);

			// Act:
			transaction.signature[0] ^= 0xFF;
			const modifiedHash = transactionExtensions.hash(codec, transaction);

			// Assert:
			expect(modifiedHash).to.not.deep.equal(hash);
		});

		it('does not change if s part of signature changes', () => {
			// Arrange:
			const transaction = createMockTransaction(12, 56);
			const hash = transactionExtensions.hash(codec, transaction);

			// Act:
			transaction.signature[sizes.signature / 2] ^= 0xFF;
			const modifiedHash = transactionExtensions.hash(codec, transaction);

			// Assert:
			expect(modifiedHash).to.deep.equal(hash);
		});

		it('changes if signer public key changes', () => {
			// Arrange:
			const transaction = createMockTransaction(12, 56);
			const hash = transactionExtensions.hash(codec, transaction);

			// Act:
			transaction.signerPublicKey[sizes.signerPublicKey / 2] ^= 0xFF;
			const modifiedHash = transactionExtensions.hash(codec, transaction);

			// Assert:
			expect(modifiedHash).to.not.deep.equal(hash);
		});

		it('changes if entity data changes', () => {
			// Arrange:
			const transaction = createMockTransaction(12, 56);
			const hash = transactionExtensions.hash(codec, transaction);

			// Act:
			++transaction.alpha;
			const modifiedHash = transactionExtensions.hash(codec, transaction);

			// Assert:
			expect(modifiedHash).to.not.deep.equal(hash);
		});
	});

	describe('sign and verify', () => {
		it('cannot validate unsigned transaction', () => {
			// Arrange:
			const transaction = createMockTransaction(12, 56);

			// Act:
			const isVerified = transactionExtensions.verify(codec, transaction);

			// Assert:
			expect(isVerified).is.equal(false);
		});

		it('can validate signed transaction', () => {
			// Arrange:
			const transaction = createMockTransaction(12, 56);
			const signerKeyPair = test.random.keyPair();
			transaction.signerPublicKey = signerKeyPair.publicKey;
			transactionExtensions.sign(codec, signerKeyPair, transaction);

			// Act:
			const isVerified = transactionExtensions.verify(codec, transaction);

			// Assert:
			expect(isVerified).is.equal(true);
		});

		it('cannot validate altered signed transaction', () => {
			// Arrange:
			const transaction = createMockTransaction(12, 56);
			const signerKeyPair = test.random.keyPair();
			transaction.signerPublicKey = signerKeyPair.publicKey;
			transactionExtensions.sign(codec, signerKeyPair, transaction);

			++transaction.alpha;

			// Act:
			const isVerified = transactionExtensions.verify(codec, transaction);

			// Assert:
			expect(isVerified).is.equal(false);
		});

		it('cannot validate signed transaction with altered signature', () => {
			// Arrange:
			const transaction = createMockTransaction(12, 56);
			const signerKeyPair = test.random.keyPair();
			transaction.signerPublicKey = signerKeyPair.publicKey;
			transactionExtensions.sign(codec, signerKeyPair, transaction);

			transaction.signature[0] ^= 0xFF;

			// Act:
			const isVerified = transactionExtensions.verify(codec, transaction);

			// Assert:
			expect(isVerified).is.equal(false);
		});
	});
});
