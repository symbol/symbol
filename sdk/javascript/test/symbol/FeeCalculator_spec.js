import { calculateTransactionFee } from '../../src/symbol/FeeCalculator.js';
import { Network } from '../../src/symbol/Network.js';
import TransactionFactory from '../../src/symbol/TransactionFactory.js';
import { expect } from 'chai';

describe('FeeCalculator', () => {
	describe('calculateTransactionFee', () => {
		it('can calculate fee for transaction without cosignatures', () => {
			// Arrange:
			const factory = new TransactionFactory(Network.TESTNET);
			const transaction = factory.create({
				type: 'transfer_transaction_v1'
			});

			// Act + Assert: transfer size is 160
			expect(calculateTransactionFee(transaction, 100)).to.equal(16000n);
			expect(calculateTransactionFee(transaction, 150)).to.equal(24000n);
			expect(calculateTransactionFee(transaction, 200)).to.equal(32000n);
		});

		it('can calculate fee for transaction with cosignatures', () => {
			// Arrange:
			const factory = new TransactionFactory(Network.TESTNET);
			const transaction = factory.create({
				type: 'transfer_transaction_v1'
			});

			// Act + Assert: transfer size is 160, cosignature size is 104
			expect(calculateTransactionFee(transaction, 100, 3)).to.equal(16000n + 312n);
			expect(calculateTransactionFee(transaction, 150, 4)).to.equal(24000n + 416n);
			expect(calculateTransactionFee(transaction, 200, 5)).to.equal(32000n + 520n);
		});
	});
});
