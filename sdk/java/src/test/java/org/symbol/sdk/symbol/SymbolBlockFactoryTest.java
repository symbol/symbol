package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsInAnyOrder;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.symbol.models.Block;
import org.symbol.sdk.symbol.models.BlockType;
import org.symbol.sdk.symbol.models.ImportanceBlockV1;
import org.symbol.sdk.symbol.models.NemesisBlockV1;
import org.symbol.sdk.symbol.models.NetworkType;
import org.symbol.sdk.symbol.models.NormalBlockV1;
import org.symbol.sdk.symbol.models.PublicKey;

/** Tests {@link SymbolBlockFactory}. */
final class SymbolBlockFactoryTest {

	private static final String SAMPLE_ADDRESS = "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y";
	private static final String SAMPLE_HASH_HEX = "0".repeat(64);
	private static final String SAMPLE_PUBLIC_KEY_HEX = "0".repeat(64);
	private static final String SAMPLE_SIGNATURE_HEX = "0".repeat(128);

	private static Map<String, Object> vrfProofDescriptor() {
		// proof pods only accept their canonical byte[] ctor — supply typed instances
		final Map<String, Object> proof = new LinkedHashMap<>();
		proof.put("gamma", new org.symbol.sdk.symbol.models.ProofGamma(new byte[org.symbol.sdk.symbol.models.ProofGamma.SIZE]));
		proof.put("verificationHash",
				new org.symbol.sdk.symbol.models.ProofVerificationHash(new byte[org.symbol.sdk.symbol.models.ProofVerificationHash.SIZE]));
		proof.put("scalar", new org.symbol.sdk.symbol.models.ProofScalar(new byte[org.symbol.sdk.symbol.models.ProofScalar.SIZE]));
		return proof;
	}

	private static Map<String, Object> baseBlockDescriptor(final String typeName) {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", typeName);
		// no "Signature" rule is registered for blocks — pass a pre-built model Signature
		descriptor.put("signature", new org.symbol.sdk.symbol.models.Signature(new byte[org.symbol.sdk.symbol.models.Signature.SIZE]));
		descriptor.put("signerPublicKey", SAMPLE_PUBLIC_KEY_HEX);
		descriptor.put("height", 1L);
		descriptor.put("timestamp", 1L);
		descriptor.put("difficulty", 1L);
		descriptor.put("generationHashProof", vrfProofDescriptor());
		descriptor.put("previousBlockHash", SAMPLE_HASH_HEX);
		descriptor.put("transactionsHash", SAMPLE_HASH_HEX);
		descriptor.put("receiptsHash", SAMPLE_HASH_HEX);
		descriptor.put("stateHash", SAMPLE_HASH_HEX);
		descriptor.put("beneficiaryAddress", SAMPLE_ADDRESS);
		descriptor.put("feeMultiplier", 1L);
		return descriptor;
	}

	@Test
	void canCreateBlockWithOverriddenRule() {
		// Arrange: override the "PublicKey" rule, so the signer must come from the override, not the descriptor hex
		final byte[] fakeSignerBytes = new byte[PublicKey.SIZE];
		Arrays.fill(fakeSignerBytes, (byte) 0x42);
		final PublicKey fakeSigner = new PublicKey(fakeSignerBytes);
		final Map<String, java.util.function.Function<Object, Object>> overrides = Map.of("PublicKey", v -> fakeSigner);
		final SymbolBlockFactory factory = new SymbolBlockFactory(Network.TESTNET, overrides);

		final Map<String, Object> descriptor = baseBlockDescriptor("normal_block_v1");
		descriptor.put("transactions", Collections.emptyList());

		// Act:
		final Block block = factory.create(descriptor);

		// Assert: the overridden rule produced the fake signer; un-overridden rules ran normally
		assertThat(block.getSignerPublicKey(), is(equalTo(fakeSigner)));
		final NormalBlockV1 typed = (NormalBlockV1) block;
		assertThat(typed.getHeight().value(), is(equalTo(1L)));
	}

	// (Java-only) registry snapshot — JS has no counterpart factory; every symbol factory registers the FULL generated
	// POD_FACTORIES registry, so types owned by the other factories appear here too
	@Test
	void hasRulesWithExpectedHints() {
		// Arrange:
		final SymbolBlockFactory factory = new SymbolBlockFactory(Network.TESTNET);

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
				"struct:VrfProof",

				// ... and the resolved-Address override (replaces the generated entry under the same name).
				"Address"
		};
		assertThat(ruleNames, containsInAnyOrder(expected));
	}

	@Test
	void createNormalBlock() {
		// Arrange:
		final Map<String, Object> descriptor = baseBlockDescriptor("normal_block_v1");
		descriptor.put("transactions", Collections.emptyList());
		final SymbolBlockFactory factory = new SymbolBlockFactory(Network.TESTNET);

		// Act:
		final Block block = factory.create(descriptor);

		// Assert:
		assertThat(block, is(instanceOf(NormalBlockV1.class)));
		final NormalBlockV1 typed = (NormalBlockV1) block;
		assertThat(typed.getType(), is(equalTo(BlockType.NORMAL)));
		assertThat(typed.getNetwork(), is(equalTo(NetworkType.TESTNET)));
		assertThat(typed.getHeight().value(), is(equalTo(1L)));
	}

	@Test
	void createRejectsAliasForResolvedBeneficiaryAddress() {
		// Arrange: beneficiaryAddress is a resolved Address field, so a namespace alias is not a valid value
		final Map<String, Object> descriptor = baseBlockDescriptor("normal_block_v1");
		descriptor.put("transactions", Collections.emptyList());
		descriptor.put("beneficiaryAddress", "THBIMC3THGH5RUYAAAAAAAAAAAAAAAAAAAAAAAA"); // low bit of first byte set -> alias
		final SymbolBlockFactory factory = new SymbolBlockFactory(Network.TESTNET);

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> factory.create(descriptor));

		// Assert:
		assertThat(ex.getMessage(), containsString("cannot be a namespace alias"));
	}

	@Test
	void createImportanceBlock() {
		// Arrange:
		final Map<String, Object> descriptor = baseBlockDescriptor("importance_block_v1");
		descriptor.put("transactions", Collections.emptyList());
		descriptor.put("votingEligibleAccountsCount", 0L);
		descriptor.put("harvestingEligibleAccountsCount", 0L);
		descriptor.put("totalVotingBalance", 0L);
		descriptor.put("previousImportanceBlockHash", SAMPLE_HASH_HEX);
		final SymbolBlockFactory factory = new SymbolBlockFactory(Network.TESTNET);

		// Act:
		final Block block = factory.create(descriptor);

		// Assert:
		assertThat(block, is(instanceOf(ImportanceBlockV1.class)));
		final ImportanceBlockV1 typed = (ImportanceBlockV1) block;
		assertThat(typed.getType(), is(equalTo(BlockType.IMPORTANCE)));
	}

	@Test
	void createNemesisBlock() {
		// Arrange:
		final Map<String, Object> descriptor = baseBlockDescriptor("nemesis_block_v1");
		descriptor.put("transactions", Collections.emptyList());
		descriptor.put("votingEligibleAccountsCount", 0L);
		descriptor.put("harvestingEligibleAccountsCount", 0L);
		descriptor.put("totalVotingBalance", 0L);
		descriptor.put("previousImportanceBlockHash", SAMPLE_HASH_HEX);
		final SymbolBlockFactory factory = new SymbolBlockFactory(Network.TESTNET);

		// Act:
		final Block block = factory.create(descriptor);

		// Assert:
		assertThat(block, is(instanceOf(NemesisBlockV1.class)));
		final NemesisBlockV1 typed = (NemesisBlockV1) block;
		assertThat(typed.getType(), is(equalTo(BlockType.NEMESIS)));
	}
}
