package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;

import java.util.LinkedHashMap;
import java.util.Map;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.models.Transaction;

/**
 * Fee Calculator tests
 */
final class FeeCalculatorTest {
	private static final CryptoTypes.PublicKey TEST_SIGNER_PUBLIC_KEY = new CryptoTypes.PublicKey(new byte[32]);

	private static Transaction createTransfer() {
		final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v1");
		descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
		return factory.create(descriptor);
	}

	@Test
	void canCalculateFeeForTransactionWithoutCosignatures() {
		// Arrange:
		final Transaction transaction = createTransfer();

		// Act:
		final long fee100 = FeeCalculator.calculateTransactionFee(transaction, 100);
		final long fee150 = FeeCalculator.calculateTransactionFee(transaction, 150);
		final long fee200 = FeeCalculator.calculateTransactionFee(transaction, 200);

		// Assert: transfer size is 160
		assertThat(fee100, equalTo(16000L));
		assertThat(fee150, equalTo(24000L));
		assertThat(fee200, equalTo(32000L));
	}

	@Test
	void canCalculateFeeForTransactionWithCosignatures() {
		// Arrange:
		final Transaction transaction = createTransfer();

		// Act:
		final long fee100 = FeeCalculator.calculateTransactionFee(transaction, 100, 3);
		final long fee150 = FeeCalculator.calculateTransactionFee(transaction, 150, 4);
		final long fee200 = FeeCalculator.calculateTransactionFee(transaction, 200, 5);

		// Assert: transfer size is 160, cosignature size is 104
		assertThat(fee100, equalTo(16000L + 312L));
		assertThat(fee150, equalTo(24000L + 416L));
		assertThat(fee200, equalTo(32000L + 520L));
	}
}
