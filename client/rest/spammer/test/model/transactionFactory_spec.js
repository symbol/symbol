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

const transactionFactory = require('../../src/model/transactionFactory');
const networkTime = require('../../src/utils/networkTime');
const test = require('../testUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');

const Signature_Size = catapult.constants.sizes.signature;
const Address_Decoded_Size = catapult.constants.sizes.addressDecoded;
const createKey = catapult.crypto.createKeyPairFromPrivateKeyString;

describe('transaction factory', () => {
	const Private_Key = '8D31B712AB28D49591EAF5066E9E967B44507FC19C3D54D742F7B3A255CFF4AB';
	const Mijin_Test_Network = catapult.model.networkInfo.networks.mijinTest.id;

	const assertTransactionData = (transaction, keyPair, version, type) => {
		const txDeadline = catapult.utils.uint64.compact(transaction.deadline);
		expect(transaction.signerPublicKey).to.deep.equal(keyPair.publicKey);
		expect(transaction.signature).to.deep.equal(new Uint8Array(Signature_Size));
		expect(transaction.version).to.equal(version);
		expect(transaction.type).to.equal(type);
		expect(transaction.maxFee).to.deep.equal([0x00, 0x00]);
		expect(txDeadline).to.be.at.least(networkTime.getNetworkTime());
		expect(txDeadline).to.be.at.most(networkTime.getNetworkTime() + (24 * 60 * 60 * 1000));
	};

	describe('random transfer', () => {
		it('can be created with reasonable data', () => {
			// Arrange:
			const keyPair = createKey(Private_Key);
			const recipientAddress = test.random.bytes(Address_Decoded_Size);

			// Act:
			const transaction = transactionFactory.createRandomTransfer(
				{ signerPublicKey: keyPair.publicKey, networkId: Mijin_Test_Network, transferId: 0x1234 },
				() => recipientAddress
			);

			// Assert:
			assertTransactionData(transaction, keyPair, Mijin_Test_Network + 3, 0x4154);
			expect(transaction.recipientAddress).to.deep.equal(recipientAddress);
			expect(transaction.message).to.deep.equal(Buffer.of(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34));
			expect(transaction.mosaics[0].id).to.deep.equal([0xD95FCF29, 0xD525AD41]);
		});
	});

	describe('aggregate transaction', () => {
		it('can be created with reasonable data', () => {
			// Arrange:
			const keyPair = createKey(Private_Key);
			const transactions = [1, 2, 3];

			// Act:
			const transaction = transactionFactory.createAggregateTransaction(
				{ signerPublicKey: keyPair.publicKey, networkId: 0xA5 },
				transactions
			);

			// Assert:
			assertTransactionData(transaction, keyPair, 0xA8, 0x4141);
			expect(transaction.transactions).to.equal(transactions);
			expect(transactions.cosignatures).to.equal(undefined);
		});
	});
});
