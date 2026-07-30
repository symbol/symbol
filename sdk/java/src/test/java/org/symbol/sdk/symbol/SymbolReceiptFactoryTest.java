package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsInAnyOrder;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.symbol.models.Amount;
import org.symbol.sdk.symbol.models.HarvestFeeReceipt;
import org.symbol.sdk.symbol.models.InflationReceipt;
import org.symbol.sdk.symbol.models.MosaicRentalFeeReceipt;
import org.symbol.sdk.symbol.models.NamespaceRentalFeeReceipt;
import org.symbol.sdk.symbol.models.Receipt;
import org.symbol.sdk.symbol.models.ReceiptType;

/** Tests {@link SymbolReceiptFactory}. */
final class SymbolReceiptFactoryTest {

	private static final String SAMPLE_ADDRESS = "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y";
	private static final long SAMPLE_MOSAIC_ID = 0x7CDF3B117A3C40CCL;
	private static final long SAMPLE_AMOUNT = 1_000_000L;

	private static Map<String, Object> mosaicDescriptor() {
		final Map<String, Object> mosaic = new LinkedHashMap<>();
		mosaic.put("mosaicId", SAMPLE_MOSAIC_ID);
		mosaic.put("amount", SAMPLE_AMOUNT);
		return mosaic;
	}

	@Test
	void canCreateReceiptWithOverriddenRule() {
		// Arrange: override the "Amount" rule, so the mosaic amount must come from the override, not the descriptor
		final Amount fakeAmount = new Amount(999L);
		final Map<String, java.util.function.Function<Object, Object>> overrides = Map.of("Amount", v -> fakeAmount);
		final SymbolReceiptFactory factory = new SymbolReceiptFactory(overrides);

		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "harvest_fee_receipt");
		descriptor.put("version", 1);
		descriptor.put("mosaic", mosaicDescriptor());
		descriptor.put("targetAddress", SAMPLE_ADDRESS);

		// Act:
		final Receipt receipt = factory.create(descriptor);

		// Assert: the overridden rule produced the fake amount; un-overridden rules ran normally
		final HarvestFeeReceipt typed = (HarvestFeeReceipt) receipt;
		assertThat(typed.getMosaic().getAmount(), is(equalTo(fakeAmount)));
		assertThat(typed.getMosaic().getMosaicId().value(), is(equalTo(SAMPLE_MOSAIC_ID)));
	}

	// (Java-only) registry snapshot — JS has no counterpart factory; every symbol factory registers the FULL generated
	// POD_FACTORIES registry, so types owned by the other factories appear here too
	@Test
	void hasRulesWithExpectedHints() {
		// Arrange:
		final SymbolReceiptFactory factory = new SymbolReceiptFactory();

		// Act:
		final Set<String> ruleNames = factory.getRuleNames();

		// Assert:
		// every generated pod/enum/flags (Models.POD_FACTORIES) ...
		final String[] expected = {
				"Amount", "BlockDuration", "BlockFeeMultiplier", "Difficulty", "FinalizationEpoch", "FinalizationPoint", "Height",
				"Importance", "ImportanceHeight", "MosaicId", "MosaicNonce", "MosaicRestrictionKey", "NamespaceId", "Timestamp",
				"UnresolvedMosaicId", "Hash512", "Signature", "ProofGamma", "ProofScalar", "ProofVerificationHash", "UnresolvedAddress",
				"Hash256", "PublicKey", "VotingPublicKey",

				"MosaicFlags", "AccountRestrictionFlags",

				"AliasAction", "BlockType", "LinkAction", "LockHashAlgorithm", "MosaicRestrictionType", "MosaicSupplyChangeAction",
				"NamespaceRegistrationType", "NetworkType", "ReceiptType", "TransactionType",

				// ... plus the factory-specific struct parser ...
				"struct:Mosaic",

				// ... and the resolved-Address override (replaces the generated entry under the same name).
				"Address"
		};
		assertThat(ruleNames, containsInAnyOrder(expected));
	}

	@Test
	void createHarvestFeeReceipt() {
		// Arrange:
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "harvest_fee_receipt");
		descriptor.put("version", 1);
		descriptor.put("mosaic", mosaicDescriptor());
		descriptor.put("targetAddress", SAMPLE_ADDRESS);
		final SymbolReceiptFactory factory = new SymbolReceiptFactory();

		// Act:
		final Receipt receipt = factory.create(descriptor);

		// Assert:
		assertThat(receipt, is(instanceOf(HarvestFeeReceipt.class)));
		final HarvestFeeReceipt typed = (HarvestFeeReceipt) receipt;
		assertThat(typed.getType(), is(equalTo(ReceiptType.HARVEST_FEE)));
		assertThat(typed.getMosaic().getAmount().value(), is(equalTo(SAMPLE_AMOUNT)));
	}

	@Test
	void createRejectsAliasForResolvedTargetAddress() {
		// Arrange: targetAddress is a resolved Address field, so a namespace alias is not a valid value
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "harvest_fee_receipt");
		descriptor.put("version", 1);
		descriptor.put("mosaic", mosaicDescriptor());
		descriptor.put("targetAddress", "THBIMC3THGH5RUYAAAAAAAAAAAAAAAAAAAAAAAA"); // low bit of first byte set -> alias
		final SymbolReceiptFactory factory = new SymbolReceiptFactory();

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> factory.create(descriptor));

		// Assert:
		assertThat(ex.getMessage(), containsString("cannot be a namespace alias"));
	}

	@Test
	void createInflationReceipt() {
		// Arrange:
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "inflation_receipt");
		descriptor.put("version", 1);
		descriptor.put("mosaic", mosaicDescriptor());
		final SymbolReceiptFactory factory = new SymbolReceiptFactory();

		// Act:
		final Receipt receipt = factory.create(descriptor);

		// Assert:
		assertThat(receipt, is(instanceOf(InflationReceipt.class)));
		final InflationReceipt typed = (InflationReceipt) receipt;
		assertThat(typed.getType(), is(equalTo(ReceiptType.INFLATION)));
	}

	@Test
	void createMosaicRentalFeeReceipt() {
		// Arrange:
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "mosaic_rental_fee_receipt");
		descriptor.put("version", 1);
		descriptor.put("mosaic", mosaicDescriptor());
		descriptor.put("senderAddress", SAMPLE_ADDRESS);
		descriptor.put("recipientAddress", SAMPLE_ADDRESS);
		final SymbolReceiptFactory factory = new SymbolReceiptFactory();

		// Act:
		final Receipt receipt = factory.create(descriptor);

		// Assert:
		assertThat(receipt, is(instanceOf(MosaicRentalFeeReceipt.class)));
		final MosaicRentalFeeReceipt typed = (MosaicRentalFeeReceipt) receipt;
		assertThat(typed.getType(), is(equalTo(ReceiptType.MOSAIC_RENTAL_FEE)));
	}

	@Test
	void createNamespaceRentalFeeReceipt() {
		// Arrange:
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "namespace_rental_fee_receipt");
		descriptor.put("version", 1);
		descriptor.put("mosaic", mosaicDescriptor());
		descriptor.put("senderAddress", SAMPLE_ADDRESS);
		descriptor.put("recipientAddress", SAMPLE_ADDRESS);
		final SymbolReceiptFactory factory = new SymbolReceiptFactory();

		// Act:
		final Receipt receipt = factory.create(descriptor);

		// Assert:
		assertThat(receipt, is(instanceOf(NamespaceRentalFeeReceipt.class)));
		final NamespaceRentalFeeReceipt typed = (NamespaceRentalFeeReceipt) receipt;
		assertThat(typed.getType(), is(equalTo(ReceiptType.NAMESPACE_RENTAL_FEE)));
	}
}
