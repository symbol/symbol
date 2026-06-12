import { calculateMosaicRentalFee, calculateNamespaceRentalFee, calculateTransactionFee } from '../../src/nem/FeeCalculator.js';
import { Network } from '../../src/nem/Network.js';
import TransactionFactory from '../../src/nem/TransactionFactory.js';
import * as nc from '../../src/nem/models.js';
import { expect } from 'chai';

describe('FeeCalculator', () => {
	describe('calculateMosaicRentalFee', () => {
		it('calculates correct fee', () => {
			expect(calculateMosaicRentalFee()).to.equal(10_000_000n);
		});
	});

	describe('calculateNamespaceRentalFee', () => {
		it('calculates correct root fee', () => {
			expect(calculateNamespaceRentalFee(true)).to.equal(100_000_000n);
		});

		it('calculates correct child fee', () => {
			expect(calculateNamespaceRentalFee(false)).to.equal(10_000_000n);
		});
	});

	describe('calculateTransactionFee', () => {
		const createTransactionWithType = type => {
			const transaction = new nc.Transaction();
			transaction.type = type;
			return transaction;
		};

		it('calculates correct fee for multisig account modification', () => {
			// Act:
			const fee = calculateTransactionFee(createTransactionWithType(nc.TransactionType.MULTISIG_ACCOUNT_MODIFICATION));

			// Assert:
			expect(fee).to.equal(500_000n);
		});

		it('calculates correct fee for other transactions', () => {
			// Arrange:
			const specialTransactionTypeNames = ['TRANSFER', 'MULTISIG_ACCOUNT_MODIFICATION'];
			const otherTransactionTypeNames = Object.getOwnPropertyNames(nc.TransactionType).filter(propertyName =>
				nc.TransactionType[propertyName].value && !specialTransactionTypeNames.includes(propertyName));

			// Sanity:
			expect(otherTransactionTypeNames.length).to.equal(6);

			// Act:
			otherTransactionTypeNames.forEach(transactionTypeName => {
				const fee = calculateTransactionFee(createTransactionWithType(nc.TransactionType[transactionTypeName]));

				// Assert:
				expect(fee, transactionTypeName).to.equal(150_000n);
			});
		});

		describe('calculates correct fee for transfers', () => {
			const weightWithFeeUnit = amount => amount * 50_000n;
			const createMosaicId = (namespaceName, name) => {
				const encoder = new TextEncoder();
				return { namespaceId: { name: encoder.encode(namespaceName) }, name: encoder.encode(name) };
			};

			describe('simple', () => {
				const assertXemFee = (amount, messageSize, expectedFee) => {
					// Arrange:
					const factory = new TransactionFactory(Network.TESTNET);

					const descriptor = {
						type: 'transfer_transaction_v2',
						amount: amount * 1_000_000
					};
					if (messageSize) {
						descriptor.message = {
							messageType: 1,
							message: 'a'.repeat(messageSize)
						};
					}

					const transaction = factory.create(descriptor);

					// Act:
					const fee = calculateTransactionFee(transaction);

					// Assert:
					expect(fee, `amount ${amount}, messageSize ${messageSize}`).to.equal(expectedFee);
				};

				it('when empty', () => {
					assertXemFee(0, 0, weightWithFeeUnit(1n));
				});

				it('near step increases', () => {
					// Arrange: fee is initially 1 and increased every 10k XEM until is reaches a max fee of 25 XEM
					const step = 10_000;
					for (let i = 0; 26 > i; ++i) {
						const amount = i * step;
						const fee = BigInt(Math.max(1, Math.min(25, amount / step)));

						// Act + Assert:
						assertXemFee(amount, 0, weightWithFeeUnit(fee));
						assertXemFee(amount + 1, 0, weightWithFeeUnit(fee));
						assertXemFee(amount + 100, 0, weightWithFeeUnit(fee));
						assertXemFee(amount + step - 1, 0, weightWithFeeUnit(fee));
					}
				});

				it('caps fee at 25 XEM', () => {
					const amounts = [250_000, 250_001, 500_000, 1_000_000, 10_000_000, 100_000_000, 1_000_000_000];
					amounts.forEach(amount => {
						assertXemFee(amount, 0, weightWithFeeUnit(25n));
					});
				});

				it('with message', () => {
					assertXemFee(10_000, 96, weightWithFeeUnit(1n + 4n));
					assertXemFee(100_000, 128, weightWithFeeUnit(10n + 5n));
					assertXemFee(1_000_000, 96, weightWithFeeUnit(25n + 4n));
					assertXemFee(2_000_000, 128, weightWithFeeUnit(25n + 5n));
				});

				it('with smallest message', () => {
					assertXemFee(1200, 1, weightWithFeeUnit(1n + 1n));
				});

				it('near message step increases', () => {
					assertXemFee(1200, 31, weightWithFeeUnit(1n + 1n));
					assertXemFee(1200, 32, weightWithFeeUnit(1n + 2n));
					assertXemFee(1200, 33, weightWithFeeUnit(1n + 2n));

					assertXemFee(1200, 63, weightWithFeeUnit(1n + 2n));
					assertXemFee(1200, 64, weightWithFeeUnit(1n + 3n));
					assertXemFee(1200, 65, weightWithFeeUnit(1n + 3n));
				});

				it('with large message', () => {
					assertXemFee(1200, 96, weightWithFeeUnit(1n + 4n));
					assertXemFee(1200, 128, weightWithFeeUnit(1n + 5n));
					assertXemFee(1200, 256, weightWithFeeUnit(1n + 9n));
					assertXemFee(1200, 320, weightWithFeeUnit(1n + 11n));
				});
			});

			describe('small business mosaics', () => {
				// A so-called small business mosaic has divisibility of 0 and a max supply of 10000
				// It is always charged 1 XEM fee no matter how many mosaics are transferred
				// Mosaic 'small business x' has divisibility 0 and supply x * 1000 for x > 0
				// Mosaic 'small business 0' has divisibility 1 and supply 1000 (so it is NOT a small business mosaic)

				const mosaicInformationLookup = mosaicId => {
					const smallBusinessPrefix = 'small business';
					if (`${smallBusinessPrefix} 0` === mosaicId.name)
						return { supply: 1000n, divisibility: 1 };

					if (mosaicId.name.startsWith(smallBusinessPrefix)) {
						const supply = BigInt(parseInt(mosaicId.name.substring(smallBusinessPrefix.length + 1), 10) * 1000);
						return { supply, divisibility: 0 };
					}

					return undefined;
				};

				const assertSmallBusinessMosaicFee = (smallBusinessId, amount, expectedFee) => {
					// Arrange:
					const factory = new TransactionFactory(Network.TESTNET);
					const transaction = factory.create({
						type: 'transfer_transaction_v2',
						amount: 1_000_000,
						mosaics: [
							{
								mosaic: {
									mosaicId: createMosaicId('foo', `small business ${smallBusinessId}`),
									amount
								}
							}
						]
					});

					// Act:
					const fee = calculateTransactionFee(transaction, mosaicInformationLookup);

					// Assert:
					expect(fee, `smallBusinessId ${smallBusinessId}`).to.equal(expectedFee);
				};

				it('uses minimum fee for mosaics with divisibility zero and low supply', () => {
					for (let i = 1; 10 >= i; ++i)
						assertSmallBusinessMosaicFee(i, i * 1000, weightWithFeeUnit(1n));
				});

				it('does not use minimum fee for mosaics with divisibility zero and supply above threshold', () => {
					// Assert: supply of 11000 means it is not a small business mosaic
					assertSmallBusinessMosaicFee(11, 1000, weightWithFeeUnit(4n));
				});

				it('does not use minimum fee for mosaics with divisibility greater than zero', () => {
					// Arrange: nonzero divisibility means it is not a small business mosaic
					assertSmallBusinessMosaicFee(0, 1000, weightWithFeeUnit(3n));
				});
			});

			describe('other mosaic', () => {
				// mosaic definition data used for the following tests: supply = 100_000_000, divisibility = 3
				// supply ratio: 8_999_999_999 / 100_000_000 ≈ 90
				// divisibility ratio = 1_000_000 / 1_000 = 1000
				// 1000 / 90 = 11.11..., so transferring a quantity of 12 is roughly like transferring 1 XEM
				// Adjustment for the fee is 9 XEM due to the lower supply and divisibility

				const mosaicInformationLookup = mosaicId => {
					const multiplier = parseInt(mosaicId.name, 10);
					const divisibilityChange = multiplier - 1;
					return { supply: 100_000_000n * BigInt(multiplier), divisibility: 3 + divisibilityChange };
				};

				const assertSingleMosaicFee = (amount, messageSize, quantity, expectedFee) => {
					// Arrange:
					const factory = new TransactionFactory(Network.TESTNET);

					const descriptor = {
						type: 'transfer_transaction_v2',
						amount: amount * 1_000_000,
						mosaics: [
							{
								mosaic: {
									mosaicId: createMosaicId('foo', '1'),
									amount: quantity
								}
							}
						]
					};
					if (messageSize) {
						descriptor.message = {
							messageType: 1,
							message: 'a'.repeat(messageSize)
						};
					}

					const transaction = factory.create(descriptor);

					// Act:
					const fee = calculateTransactionFee(transaction, mosaicInformationLookup);

					// Assert:
					expect(fee, `amount ${amount}, messageSize ${messageSize}, quantity ${quantity}`).to.equal(expectedFee);
				};

				it('near mosaic transfer step increases', () => {
					// Assert: minimum fee for low amounts
					assertSingleMosaicFee(1, 0, 12, weightWithFeeUnit(1n)); // ~ 1 XEM
					assertSingleMosaicFee(1, 0, 111_000, weightWithFeeUnit(1n)); // ~9_999 XEM

					// - 1 -> 2 roughly at 1222.222 units
					assertSingleMosaicFee(1, 0, 1_222_000n, weightWithFeeUnit(1n));
					assertSingleMosaicFee(1, 0, 1_223_000n, weightWithFeeUnit(2n)); // ~ 110_000 XEM
					assertSingleMosaicFee(1, 0, 1_224_000n, weightWithFeeUnit(2n));

					// - 2 -> 3 roughly at 1333.333 units
					assertSingleMosaicFee(1, 0, 1_333_000n, weightWithFeeUnit(2n));
					assertSingleMosaicFee(1, 0, 1_334_000n, weightWithFeeUnit(3n)); // ~ 120_000 XEM
					assertSingleMosaicFee(1, 0, 1_335_000n, weightWithFeeUnit(3n));

					// - 3 -> 4 roughly at 1444.444 units
					assertSingleMosaicFee(1, 0, 1_444_000n, weightWithFeeUnit(3n));
					assertSingleMosaicFee(1, 0, 1_445_000n, weightWithFeeUnit(4n)); // ~ 130_000 XEM
					assertSingleMosaicFee(1, 0, 1_446_000n, weightWithFeeUnit(4n));
				});

				it('large mosaic transfers', () => {
					assertSingleMosaicFee(1, 0, 2_112_000n, weightWithFeeUnit(10n)); // ~ 190_000 XEM
					assertSingleMosaicFee(1, 0, 2_445_000n, weightWithFeeUnit(13n)); // ~ 220_000 XEM
					assertSingleMosaicFee(1, 0, 2_778_000n, weightWithFeeUnit(16n)); // ~ 250_000 XEM
					assertSingleMosaicFee(1, 0, 3_000_000n, weightWithFeeUnit(16n));
					assertSingleMosaicFee(1, 0, 10_000_000n, weightWithFeeUnit(16n));
					assertSingleMosaicFee(1, 0, 100_000_000n, weightWithFeeUnit(16n));
				});

				it('with amounts greater than one', () => {
					// Assert: notice that amount * quantity is constant
					assertSingleMosaicFee(1, 0, 2_112_000n, weightWithFeeUnit(10n));
					assertSingleMosaicFee(2, 0, 1_056_000n, weightWithFeeUnit(10n));
					assertSingleMosaicFee(5, 0, 422_400n, weightWithFeeUnit(10n));
					assertSingleMosaicFee(10, 0, 211_200n, weightWithFeeUnit(10n));
					assertSingleMosaicFee(100, 0, 21_120n, weightWithFeeUnit(10n));
					assertSingleMosaicFee(1_000, 0, 2_112n, weightWithFeeUnit(10n));
					assertSingleMosaicFee(21_120, 0, 100n, weightWithFeeUnit(10n));
					assertSingleMosaicFee(2_112_000, 0, 1n, weightWithFeeUnit(10n));
				});

				it('with message', () => {
					assertSingleMosaicFee(1, 15, 2_112_000n, weightWithFeeUnit(10n + 1n));
					assertSingleMosaicFee(1, 32, 2_112_000n, weightWithFeeUnit(10n + 2n));
					assertSingleMosaicFee(1, 96, 2_112_000n, weightWithFeeUnit(10n + 4n));
					assertSingleMosaicFee(1, 160, 2_112_000n, weightWithFeeUnit(10n + 6n));
				});

				it('sums fees when transferring several mosaics (function based lookup)', () => {
					// Arrange: mosaic definitions are (100M, 3), (200M, 4), (300M, 5)
					const factory = new TransactionFactory(Network.TESTNET);
					const transaction = factory.create({
						type: 'transfer_transaction_v2',
						amount: 1_000_000,
						mosaics: [2_000_000n, 50_000_000n, 800_000_000n].map((amount, i) => ({
							mosaic: {
								mosaicId: createMosaicId('foo', (i + 1).toString()),
								amount
							}
						}))
					});

					// Act:
					const fee = calculateTransactionFee(transaction, mosaicInformationLookup);

					// Assert:
					expect(fee).to.equal(weightWithFeeUnit(8n + 16n + 19n));
				});

				it('sums fees when transferring several mosaics (object based lookup)', () => {
					// Arrange: mosaic definitions are (100M, 3), (200M, 4), (300M, 5)
					const factory = new TransactionFactory(Network.TESTNET);
					const transaction = factory.create({
						type: 'transfer_transaction_v2',
						amount: 1_000_000,
						mosaics: [2_000_000n, 50_000_000n, 800_000_000n].map((amount, i) => ({
							mosaic: {
								mosaicId: createMosaicId('foo', (i + 1).toString()),
								amount
							}
						}))
					});

					// Act:
					const fee = calculateTransactionFee(transaction, {
						'foo:1': { supply: 100_000_000n, divisibility: 3 },
						'foo:2': { supply: 200_000_000n, divisibility: 4 },
						'foo:3': { supply: 300_000_000n, divisibility: 5 }
					});

					// Assert:
					expect(fee).to.equal(weightWithFeeUnit(8n + 16n + 19n));
				});
			});

			describe('edge case', () => {
				it('uses minimum fee when mosaic supply is zero', () => {
					// Arrange:
					const factory = new TransactionFactory(Network.TESTNET);
					const transaction = factory.create({
						type: 'transfer_transaction_v2',
						amount: 1_000_000,
						mosaics: [
							{
								mosaic: {
									mosaicId: createMosaicId('foo', 'zero supply'),
									amount: 5000000
								}
							}
						]
					});

					// Act:
					const fee = calculateTransactionFee(transaction, () => ({ supply: 0n, divisibility: 3 }));

					// Assert:
					expect(fee).to.equal(weightWithFeeUnit(1n));
				});

				it('fails for unknown mosaic', () => {
					// Arrange:
					const factory = new TransactionFactory(Network.TESTNET);
					const transaction = factory.create({
						type: 'transfer_transaction_v2',
						amount: 1_000_000,
						mosaics: [
							{
								mosaic: {
									mosaicId: createMosaicId('foo', 'bar'),
									amount: 5000000
								}
							}
						]
					});

					// Act + Assert:
					expect(() => calculateTransactionFee(transaction, () => undefined))
						.to.throw('unable to find fee information for foo:bar');
				});
			});
		});
	});
});
