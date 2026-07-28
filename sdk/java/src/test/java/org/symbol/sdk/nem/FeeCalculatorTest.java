package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.nem.FeeCalculator.MosaicInformation;
import org.symbol.sdk.nem.FeeCalculator.MosaicName;
import org.symbol.sdk.nem.models.Transaction;
import org.symbol.sdk.nem.models.TransactionType;

/**
 * Fee Calculator tests.
 */
final class FeeCalculatorTest {
	private static final CryptoTypes.PublicKey TEST_SIGNER_PUBLIC_KEY = new CryptoTypes.PublicKey(new byte[32]);

	private static long weightWithFeeUnit(final long amount) {
		return amount * 50_000L;
	}

	// region rental fees

	@Nested
	class CalculateMosaicRentalFee {
		@Test
		void calculatesCorrectFee() {
			// Act:
			final long fee = FeeCalculator.calculateMosaicRentalFee();

			// Assert:
			assertThat(fee, equalTo(10_000_000L));
		}
	}

	@Nested
	class CalculateNamespaceRentalFee {
		@Test
		void calculatesCorrectRootFee() {
			// Act:
			final long fee = FeeCalculator.calculateNamespaceRentalFee(true);

			// Assert:
			assertThat(fee, equalTo(100_000_000L));
		}

		@Test
		void calculatesCorrectChildFee() {
			// Act:
			final long fee = FeeCalculator.calculateNamespaceRentalFee(false);

			// Assert:
			assertThat(fee, equalTo(10_000_000L));
		}
	}

	// endregion

	// region calculateTransactionFee

	@Nested
	class CalculateTransactionFee {
		private Transaction createTransactionWithType(final TransactionType type) {
			final Transaction transaction = new Transaction();
			transaction.setType(type);
			return transaction;
		}

		@Test
		void calculatesCorrectFeeForMultisigAccountModification() {
			// Arrange:
			final Transaction transaction = createTransactionWithType(TransactionType.MULTISIG_ACCOUNT_MODIFICATION);

			// Act:
			final long fee = FeeCalculator.calculateTransactionFee(transaction);

			// Assert:
			assertThat(fee, equalTo(500_000L));
		}

		@Test
		void calculatesCorrectFeeForOtherTransactions() {
			// Arrange:
			final List<TransactionType> otherTransactionTypes = Arrays.stream(TransactionType.values())
					.filter(type -> TransactionType.TRANSFER != type && TransactionType.MULTISIG_ACCOUNT_MODIFICATION != type).toList();

			// Sanity:
			assertThat(otherTransactionTypes.size(), equalTo(6));

			for (final TransactionType type : otherTransactionTypes) {
				// Act:
				final long fee = FeeCalculator.calculateTransactionFee(createTransactionWithType(type));

				// Assert:
				assertThat(type.toString(), fee, equalTo(150_000L));
			}
		}

		@Nested
		class Transfers {
			private Map<String, Object> mosaicIdDescriptor(final String namespaceName, final String name) {
				return Map.of("namespaceId", Map.of("name", namespaceName), "name", name);
			}

			private Transaction createTransfer(final long amountMicroXem, final int messageSize, final List<Map<String, Object>> mosaics) {
				final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);
				final Map<String, Object> descriptor = new LinkedHashMap<>();
				descriptor.put("type", "transfer_transaction_v2");
				descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
				descriptor.put("amount", amountMicroXem);
				if (0 != messageSize)
					descriptor.put("message", Map.of("messageType", 1, "message", "a".repeat(messageSize)));

				if (null != mosaics)
					descriptor.put("mosaics", mosaics);

				return factory.create(descriptor);
			}

			@Nested
			class Simple {
				private void assertXemFee(final long amount, final int messageSize, final long expectedFee) {
					// Arrange:
					final Transaction transaction = createTransfer(amount * 1_000_000L, messageSize, null);

					// Act:
					final long fee = FeeCalculator.calculateTransactionFee(transaction);

					// Assert:
					assertThat(String.format("amount %d, messageSize %d", amount, messageSize), fee, equalTo(expectedFee));
				}

				@Test
				void whenEmpty() {
					assertXemFee(0, 0, weightWithFeeUnit(1L));
				}

				@Test
				void nearStepIncreases() {
					// Arrange: fee is initially 1 and increased every 10k XEM until it reaches a max fee of 25 XEM
					final long step = 10_000L;
					for (long i = 0; 26 > i; ++i) {
						final long amount = i * step;
						final long fee = Math.max(1L, Math.min(25L, amount / step));

						// Act + Assert:
						assertXemFee(amount, 0, weightWithFeeUnit(fee));
						assertXemFee(amount + 1, 0, weightWithFeeUnit(fee));
						assertXemFee(amount + 100, 0, weightWithFeeUnit(fee));
						assertXemFee(amount + step - 1, 0, weightWithFeeUnit(fee));
					}
				}

				@Test
				void capsFeeAt25Xem() {
					final long[] amounts = {
							250_000L, 250_001L, 500_000L, 1_000_000L, 10_000_000L, 100_000_000L, 1_000_000_000L
					};
					for (final long amount : amounts)
						assertXemFee(amount, 0, weightWithFeeUnit(25L));
				}

				@Test
				void withMessage() {
					assertXemFee(10_000, 96, weightWithFeeUnit(1L + 4L));
					assertXemFee(100_000, 128, weightWithFeeUnit(10L + 5L));
					assertXemFee(1_000_000, 96, weightWithFeeUnit(25L + 4L));
					assertXemFee(2_000_000, 128, weightWithFeeUnit(25L + 5L));
				}

				@Test
				void withSmallestMessage() {
					assertXemFee(1200, 1, weightWithFeeUnit(1L + 1L));
				}

				@Test
				void nearMessageStepIncreases() {
					assertXemFee(1200, 31, weightWithFeeUnit(1L + 1L));
					assertXemFee(1200, 32, weightWithFeeUnit(1L + 2L));
					assertXemFee(1200, 33, weightWithFeeUnit(1L + 2L));

					assertXemFee(1200, 63, weightWithFeeUnit(1L + 2L));
					assertXemFee(1200, 64, weightWithFeeUnit(1L + 3L));
					assertXemFee(1200, 65, weightWithFeeUnit(1L + 3L));
				}

				@Test
				void withLargeMessage() {
					assertXemFee(1200, 96, weightWithFeeUnit(1L + 4L));
					assertXemFee(1200, 128, weightWithFeeUnit(1L + 5L));
					assertXemFee(1200, 256, weightWithFeeUnit(1L + 9L));
					assertXemFee(1200, 320, weightWithFeeUnit(1L + 11L));
				}
			}

			@Nested
			class SmallBusinessMosaics {
				// A so-called small business mosaic has divisibility of 0 and a max supply of 10000
				// It is always charged 1 XEM fee no matter how many mosaics are transferred
				// Mosaic 'small business x' has divisibility 0 and supply x * 1000 for x > 0
				// Mosaic 'small business 0' has divisibility 1 and supply 1000 (so it is NOT a small business mosaic)

				private MosaicInformation lookupMosaicInformation(final MosaicName mosaicName) {
					final String smallBusinessPrefix = "small business";
					if ((smallBusinessPrefix + " 0").equals(mosaicName.name()))
						return new MosaicInformation(1000L, 1);

					if (mosaicName.name().startsWith(smallBusinessPrefix))
						return new MosaicInformation(Long.parseLong(mosaicName.name().substring(smallBusinessPrefix.length() + 1)) * 1000L,
								0);

					return null;
				}

				private void assertSmallBusinessMosaicFee(final int smallBusinessId, final long amount, final long expectedFee) {
					// Arrange:
					final Transaction transaction = createTransfer(1_000_000L, 0, List.of(Map.of("mosaic",
							Map.of("mosaicId", mosaicIdDescriptor("foo", "small business " + smallBusinessId), "amount", amount))));

					// Act:
					final long fee = FeeCalculator.calculateTransactionFee(transaction, this::lookupMosaicInformation);

					// Assert:
					assertThat("smallBusinessId " + smallBusinessId, fee, equalTo(expectedFee));
				}

				@Test
				void usesMinimumFeeForMosaicsWithDivisibilityZeroAndLowSupply() {
					for (int i = 1; 10 >= i; ++i)
						assertSmallBusinessMosaicFee(i, i * 1000L, weightWithFeeUnit(1L));
				}

				@Test
				void doesNotUseMinimumFeeForMosaicsWithDivisibilityZeroAndSupplyAboveThreshold() {
					// Assert: supply of 11000 means it is not a small business mosaic
					assertSmallBusinessMosaicFee(11, 1000L, weightWithFeeUnit(4L));
				}

				@Test
				void doesNotUseMinimumFeeForMosaicsWithDivisibilityGreaterThanZero() {
					// Arrange: nonzero divisibility means it is not a small business mosaic
					assertSmallBusinessMosaicFee(0, 1000L, weightWithFeeUnit(3L));
				}
			}

			@Nested
			class OtherMosaic {
				// mosaic definition data used for the following tests: supply = 100_000_000, divisibility = 3
				// supply ratio: 8_999_999_999 / 100_000_000 ~= 90
				// divisibility ratio = 1_000_000 / 1_000 = 1000
				// 1000 / 90 = 11.11..., so transferring a quantity of 12 is roughly like transferring 1 XEM
				// Adjustment for the fee is 9 XEM due to the lower supply and divisibility

				private MosaicInformation lookupMosaicInformation(final MosaicName mosaicName) {
					final int multiplier = Integer.parseInt(mosaicName.name());
					final int divisibilityChange = multiplier - 1;
					return new MosaicInformation(100_000_000L * multiplier, 3 + divisibilityChange);
				}

				private void assertSingleMosaicFee(final long amount, final int messageSize, final long quantity, final long expectedFee) {
					// Arrange:
					final Transaction transaction = createTransfer(amount * 1_000_000L, messageSize,
							List.of(Map.of("mosaic", Map.of("mosaicId", mosaicIdDescriptor("foo", "1"), "amount", quantity))));

					// Act:
					final long fee = FeeCalculator.calculateTransactionFee(transaction, this::lookupMosaicInformation);

					// Assert:
					assertThat(String.format("amount %d, messageSize %d, quantity %d", amount, messageSize, quantity), fee,
							equalTo(expectedFee));
				}

				@Test
				void nearMosaicTransferStepIncreases() {
					// Assert: minimum fee for low amounts
					assertSingleMosaicFee(1, 0, 12L, weightWithFeeUnit(1L)); // ~ 1 XEM
					assertSingleMosaicFee(1, 0, 111_000L, weightWithFeeUnit(1L)); // ~9_999 XEM

					// - 1 -> 2 roughly at 1222.222 units
					assertSingleMosaicFee(1, 0, 1_222_000L, weightWithFeeUnit(1L));
					assertSingleMosaicFee(1, 0, 1_223_000L, weightWithFeeUnit(2L)); // ~ 110_000 XEM
					assertSingleMosaicFee(1, 0, 1_224_000L, weightWithFeeUnit(2L));

					// - 2 -> 3 roughly at 1333.333 units
					assertSingleMosaicFee(1, 0, 1_333_000L, weightWithFeeUnit(2L));
					assertSingleMosaicFee(1, 0, 1_334_000L, weightWithFeeUnit(3L)); // ~ 120_000 XEM
					assertSingleMosaicFee(1, 0, 1_335_000L, weightWithFeeUnit(3L));

					// - 3 -> 4 roughly at 1444.444 units
					assertSingleMosaicFee(1, 0, 1_444_000L, weightWithFeeUnit(3L));
					assertSingleMosaicFee(1, 0, 1_445_000L, weightWithFeeUnit(4L)); // ~ 130_000 XEM
					assertSingleMosaicFee(1, 0, 1_446_000L, weightWithFeeUnit(4L));
				}

				@Test
				void largeMosaicTransfers() {
					assertSingleMosaicFee(1, 0, 2_112_000L, weightWithFeeUnit(10L)); // ~ 190_000 XEM
					assertSingleMosaicFee(1, 0, 2_445_000L, weightWithFeeUnit(13L)); // ~ 220_000 XEM
					assertSingleMosaicFee(1, 0, 2_778_000L, weightWithFeeUnit(16L)); // ~ 250_000 XEM
					assertSingleMosaicFee(1, 0, 3_000_000L, weightWithFeeUnit(16L));
					assertSingleMosaicFee(1, 0, 10_000_000L, weightWithFeeUnit(16L));
					assertSingleMosaicFee(1, 0, 100_000_000L, weightWithFeeUnit(16L));
				}

				@Test
				void withAmountsGreaterThanOne() {
					// Assert: notice that amount * quantity is constant
					assertSingleMosaicFee(1, 0, 2_112_000L, weightWithFeeUnit(10L));
					assertSingleMosaicFee(2, 0, 1_056_000L, weightWithFeeUnit(10L));
					assertSingleMosaicFee(5, 0, 422_400L, weightWithFeeUnit(10L));
					assertSingleMosaicFee(10, 0, 211_200L, weightWithFeeUnit(10L));
					assertSingleMosaicFee(100, 0, 21_120L, weightWithFeeUnit(10L));
					assertSingleMosaicFee(1_000, 0, 2_112L, weightWithFeeUnit(10L));
					assertSingleMosaicFee(21_120, 0, 100L, weightWithFeeUnit(10L));
					assertSingleMosaicFee(2_112_000, 0, 1L, weightWithFeeUnit(10L));
				}

				@Test
				void withMessage() {
					assertSingleMosaicFee(1, 15, 2_112_000L, weightWithFeeUnit(10L + 1L));
					assertSingleMosaicFee(1, 32, 2_112_000L, weightWithFeeUnit(10L + 2L));
					assertSingleMosaicFee(1, 96, 2_112_000L, weightWithFeeUnit(10L + 4L));
					assertSingleMosaicFee(1, 160, 2_112_000L, weightWithFeeUnit(10L + 6L));
				}

				private Transaction createMultiMosaicTransfer() {
					final long[] amounts = {
							2_000_000L, 50_000_000L, 800_000_000L
					};
					final List<Map<String, Object>> mosaics = new ArrayList<>();
					for (int i = 0; i < amounts.length; ++i)
						mosaics.add(Map.of("mosaic",
								Map.of("mosaicId", mosaicIdDescriptor("foo", Integer.toString(i + 1)), "amount", amounts[i])));

					return createTransfer(1_000_000L, 0, mosaics);
				}

				@Test
				void sumsFeesWhenTransferringSeveralMosaicsFunctionBasedLookup() {
					// Arrange: mosaic definitions are (100M, 3), (200M, 4), (300M, 5)
					final Transaction transaction = createMultiMosaicTransfer();

					// Act:
					final long fee = FeeCalculator.calculateTransactionFee(transaction, this::lookupMosaicInformation);

					// Assert:
					assertThat(fee, equalTo(weightWithFeeUnit(8L + 16L + 19L)));
				}

				@Test
				void sumsFeesWhenTransferringSeveralMosaicsObjectBasedLookup() {
					// Arrange: mosaic definitions are (100M, 3), (200M, 4), (300M, 5)
					final Transaction transaction = createMultiMosaicTransfer();

					// Act:
					final long fee = FeeCalculator.calculateTransactionFee(transaction,
							Map.of("foo:1", new MosaicInformation(100_000_000L, 3), "foo:2", new MosaicInformation(200_000_000L, 4),
									"foo:3", new MosaicInformation(300_000_000L, 5)));

					// Assert:
					assertThat(fee, equalTo(weightWithFeeUnit(8L + 16L + 19L)));
				}
			}

			@Nested
			class EdgeCase {
				@Test
				void usesMinimumFeeWhenMosaicSupplyIsZero() {
					// Arrange:
					final Transaction transaction = createTransfer(1_000_000L, 0,
							List.of(Map.of("mosaic", Map.of("mosaicId", mosaicIdDescriptor("foo", "zero supply"), "amount", 5_000_000L))));

					// Act:
					final long fee = FeeCalculator.calculateTransactionFee(transaction, mosaicName -> new MosaicInformation(0L, 3));

					// Assert:
					assertThat(fee, equalTo(weightWithFeeUnit(1L)));
				}

				@Test
				void failsForUnknownMosaic() {
					// Arrange:
					final Transaction transaction = createTransfer(1_000_000L, 0,
							List.of(Map.of("mosaic", Map.of("mosaicId", mosaicIdDescriptor("foo", "bar"), "amount", 5_000_000L))));

					// Act:
					final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
							() -> FeeCalculator.calculateTransactionFee(transaction, mosaicName -> null));

					// Assert:
					assertThat(ex.getMessage(), containsString("unable to find fee information for foo:bar"));
				}

				@Test
				void failsForMosaicTotalQuantityExceedingMaximum() {
					// Arrange: supply 1e9 at divisibility 8 -> total quantity 1e17 > 9e15
					final Transaction transaction = createTransfer(1_000_000L, 0,
							List.of(Map.of("mosaic", Map.of("mosaicId", mosaicIdDescriptor("foo", "bar"), "amount", 5_000_000L))));

					// Act:
					final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> FeeCalculator
							.calculateTransactionFee(transaction, mosaicName -> new MosaicInformation(1_000_000_000L, 8)));

					// Assert:
					assertThat(ex.getMessage(), containsString("exceeds"));
				}
			}
		}
	}

	// endregion
}
