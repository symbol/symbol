package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsInAnyOrder;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.startsWith;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Function;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.nem.models.*;

/**
 * Tests {@link NemTransactionFactory}: rule registration, create, non-verifiable conversion, and serialize round-trips.
 */
final class NemTransactionFactoryTest {

	private static final byte[] TEST_SIGNER_PUBLIC_KEY_BYTES = new byte[CryptoTypes.PublicKey.SIZE];

	static {
		for (int i = 0; i < TEST_SIGNER_PUBLIC_KEY_BYTES.length; ++i)
			TEST_SIGNER_PUBLIC_KEY_BYTES[i] = (byte) (i + 1);
	}

	private static final CryptoTypes.PublicKey TEST_SIGNER_PUBLIC_KEY = new CryptoTypes.PublicKey(TEST_SIGNER_PUBLIC_KEY_BYTES);

	private static Map<String, Object> transferDescriptor() {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v1");
		descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
		descriptor.put("fee", 1000L);
		descriptor.put("timestamp", 100L);
		descriptor.put("deadline", 200L);
		descriptor.put("recipientAddress", new Address("TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C"));
		descriptor.put("amount", 5L);
		return descriptor;
	}

	// region constants + rules

	@Test
	void hasRulesWithExpectedHints() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);

		// Act:
		final Set<String> ruleNames = factory.getRuleNames();

		// Assert:
		// every generated pod/enum (Models.FACTORIES) ...
		final String[] expected = {
				"Amount", "Height", "Timestamp", "Signature",

				"LinkAction", "MessageType", "MosaicSupplyChangeAction", "MosaicTransferFeeType", "MultisigAccountModificationType",
				"NetworkType", "TransactionType",

				// ... plus the registered struct parsers ...
				"struct:Message", "struct:NamespaceId", "struct:MosaicId", "struct:Mosaic", "struct:SizePrefixedMosaic",
				"struct:MosaicLevy", "struct:MosaicProperty", "struct:SizePrefixedMosaicProperty", "struct:MosaicDefinition",
				"struct:MultisigAccountModification", "struct:SizePrefixedMultisigAccountModification", "struct:CosignatureV1",
				"struct:SizePrefixedCosignatureV1",

				// ... the SDK pod overrides (replace the generated entries under the same names) ...
				"Address", "Hash256", "PublicKey",

				// ... and the array parsers.
				"array[SizePrefixedMosaic]", "array[SizePrefixedMosaicProperty]", "array[SizePrefixedMultisigAccountModification]",
				"array[SizePrefixedCosignatureV1]"
		};
		assertThat(ruleNames, containsInAnyOrder(expected));
	}

	@Test
	void getRuleNamesReturnsImmutableSnapshot() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);

		// Act:
		final Set<String> ruleNames = factory.getRuleNames();

		// Assert: the returned set is a snapshot — mutating it must fail rather than write through to the factory rules
		assertThrows(UnsupportedOperationException.class, () -> ruleNames.remove("Amount"));
		assertThat(factory.getRuleNames().contains("Amount"), is(true));
	}

	// endregion

	// region lookupTransactionName

	@Test
	void canLookupKnownTransaction() {
		// Act + Assert:
		assertThat(NemTransactionFactory.lookupTransactionName(TransactionType.TRANSFER, 1), is(equalTo("transfer_transaction_v1")));
		assertThat(NemTransactionFactory.lookupTransactionName(TransactionType.TRANSFER, 2), is(equalTo("transfer_transaction_v2")));
		assertThat(NemTransactionFactory.lookupTransactionName(TransactionType.MOSAIC_DEFINITION, 1),
				is(equalTo("mosaic_definition_transaction_v1")));
	}

	@Test
	void cannotLookupUnknownTransaction() {
		// Act + Assert: the enum cannot hold arbitrary unknown values — exercise fromValue directly
		assertThrows(IllegalArgumentException.class, () -> TransactionType.fromValue(123));
	}

	// endregion

	// region create

	@Test
	void canCreateKnownTransactionFromDescriptor() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);

		// Act:
		final Transaction transaction = factory.create(transferDescriptor());

		// Assert:
		assertThat(transaction.getType(), is(TransactionType.TRANSFER));
		assertThat(transaction.getVersion(), is(1));
		assertThat(transaction.getNetwork(), is(NetworkType.TESTNET));
		assertThat(transaction.getFee(), is(equalTo(new Amount(1000L))));
	}

	@Test
	void cannotCreateUnknownTransactionFromDescriptor() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);
		final Map<String, Object> descriptor = transferDescriptor();
		descriptor.put("type", "xtransfer_transaction_v1");

		// Act + Assert:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> factory.create(descriptor));
		assertThat(ex.getMessage(), is(equalTo("unknown Transaction type xtransfer_transaction_v1")));
	}

	@Test
	void canCreateKnownTransactionWithMultipleOverrides() {
		// Arrange: overrides are keyed by rule name and must produce model-typed values (unlike JS, which keys by class)
		final org.symbol.sdk.nem.models.Address fakeFeeSink = new org.symbol.sdk.nem.models.Address(
				"TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C".getBytes());
		final PublicKey fakeSigner = new PublicKey(TEST_SIGNER_PUBLIC_KEY.bytes());
		final Map<String, Function<Object, Object>> typeRuleOverrides = new HashMap<>();
		typeRuleOverrides.put("Address", value -> fakeFeeSink);
		typeRuleOverrides.put("Amount", value -> new Amount(654321L));
		typeRuleOverrides.put("PublicKey", value -> fakeSigner);
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET, typeRuleOverrides);

		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "namespace_registration_transaction_v1");
		descriptor.put("signerPublicKey", "signerName");
		descriptor.put("rentalFeeSink", "fee sink");
		descriptor.put("rentalFee", "fake fee");

		// Act:
		final NamespaceRegistrationTransactionV1 transaction = (NamespaceRegistrationTransactionV1) factory.create(descriptor);

		// Assert: the overridden rules produced the fake values
		assertThat(transaction.getType(), is(TransactionType.NAMESPACE_REGISTRATION));
		assertThat(transaction.getVersion(), is(1));
		assertThat(transaction.getNetwork(), is(NetworkType.TESTNET));
		assertThat(transaction.getSignerPublicKey(), is(equalTo(fakeSigner)));
		assertThat(transaction.getRentalFeeSink(), is(equalTo(fakeFeeSink)));
		assertThat(transaction.getRentalFee(), is(equalTo(new Amount(654321L))));
	}

	// endregion

	// region address type conversion

	@Test
	void canCreateTransactionWithAddress() {
		// Arrange: validates the (25-byte raw) facade Address → (40-byte base32 ASCII) model Address type-converter.
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "namespace_registration_transaction_v1");
		descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
		descriptor.put("rentalFeeSink", new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGAB"));

		// Act:
		final NamespaceRegistrationTransactionV1 transaction = (NamespaceRegistrationTransactionV1) factory.create(descriptor);

		// Assert: the model address holds the ASCII bytes of the base32 string
		assertThat(transaction.getRentalFeeSink(),
				is(equalTo(new org.symbol.sdk.nem.models.Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGAB".getBytes()))));
	}

	// endregion

	// region sorting

	private static Map<String, Object> unorderedModificationsDescriptor() {
		// modifications deliberately out of order: DELETE (type 2) before ADD (type 1).
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "multisig_account_modification_transaction_v2");
		descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
		descriptor.put("modifications", List.of(
				Map.of("modification",
						Map.of("modificationType", "delete_cosignatory", "cosignatoryPublicKey",
								new CryptoTypes.PublicKey("D79936328C188A4416224ABABF580CA2C5C8D852248DB1933FE4BC0DCA0EE7BC"))),
				Map.of("modification", Map.of("modificationType", "add_cosignatory", "cosignatoryPublicKey",
						new CryptoTypes.PublicKey("5D378657691CAD70CE35A46FB88CB134232B0B6B3655449C019A1F5F20AE9AAD")))));
		return descriptor;
	}

	@Test
	void canCreateTransactionWithOutOfOrderArrayWhenAutosortIsEnabled() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);

		// Act:
		final MultisigAccountModificationTransactionV2 transaction = (MultisigAccountModificationTransactionV2) factory
				.create(unorderedModificationsDescriptor());

		// Assert: modifications were reordered (ADD sorts before DELETE).
		assertThat(transaction.getModifications().get(0).getModification().getModificationType(),
				is(MultisigAccountModificationType.ADD_COSIGNATORY));
		assertThat(transaction.getModifications().get(1).getModification().getModificationType(),
				is(MultisigAccountModificationType.DELETE_COSIGNATORY));
	}

	@Test
	void cannotCreateTransactionWithOutOfOrderArrayWhenAutosortIsDisabled() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);

		// Act:
		final MultisigAccountModificationTransactionV2 transaction = (MultisigAccountModificationTransactionV2) factory
				.create(unorderedModificationsDescriptor(), false);

		// Assert: modifications were NOT reordered, and serialization rejects the unsorted array.
		assertThat(transaction.getModifications().get(0).getModification().getModificationType(),
				is(MultisigAccountModificationType.DELETE_COSIGNATORY));
		assertThat(transaction.getModifications().get(1).getModification().getModificationType(),
				is(MultisigAccountModificationType.ADD_COSIGNATORY));
		assertThrows(IllegalArgumentException.class, transaction::serialize);
	}

	// endregion

	// region message encoding

	@Test
	void canCreateTransferWithStringMessage() {
		// Arrange: a raw String message must be auto-encoded to its UTF-8 bytes.
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);
		final String messageText = "You miss 100%% of the shots you don't take";
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v2");
		descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
		descriptor.put("message", Map.of("messageType", "plain", "message", messageText));

		// Act:
		final TransferTransactionV2 transaction = (TransferTransactionV2) factory.create(descriptor);

		// Assert:
		assertThat(transaction.getMessage().get().getMessage(), is(equalTo(messageText.getBytes())));
	}

	// endregion

	// region toNonVerifiableTransaction

	@Test
	void canConvertVerifiableTransactionToNonVerifiable() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);
		final Transaction transaction = factory.create(transferDescriptor());

		// Act:
		final NonVerifiableTransaction nonVerifiable = NemTransactionFactory.toNonVerifiableTransaction(transaction);

		// Assert: instance is the NonVerifiable sibling of the source class, key fields are copied
		assertThat(nonVerifiable, is(org.hamcrest.Matchers.instanceOf(NonVerifiableTransferTransactionV1.class)));
		final NonVerifiableTransferTransactionV1 cast = (NonVerifiableTransferTransactionV1) nonVerifiable;
		assertThat(cast.getType(), is(TransactionType.TRANSFER));
		assertThat(cast.getFee(), is(equalTo(new Amount(1000L))));
		assertThat(cast.getAmount(), is(equalTo(new Amount(5L))));
	}

	@Test
	void canConvertNonVerifiableTransactionToNonVerifiable() {
		// Arrange:
		final NonVerifiableTransferTransactionV1 nonVerifiable = new NonVerifiableTransferTransactionV1();

		// Act:
		final NonVerifiableTransaction result = NemTransactionFactory.toNonVerifiableTransaction(nonVerifiable);

		// Assert: passing a NonVerifiable instance back through the converter yields a fresh
		// NonVerifiable of the same type.
		assertThat(result, is(org.hamcrest.Matchers.instanceOf(NonVerifiableTransferTransactionV1.class)));
	}

	@Test
	void cannotConvertNonTransactionToNonVerifiable() {
		assertThrows(IllegalArgumentException.class, () -> NemTransactionFactory.toNonVerifiableTransaction("not a transaction"));
	}

	// endregion

	// region serialize round-trip + JSON

	@Test
	void canDeserializeTransactionFromBuffer() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);
		final Transaction original = factory.create(transferDescriptor());
		final byte[] payload = original.serialize();

		// Act:
		final Transaction deserialized = NemTransactionFactory.deserialize(payload);

		// Assert:
		assertThat(deserialized.getType(), is(TransactionType.TRANSFER));
		assertThat(deserialized.serialize(), is(equalTo(payload)));
	}

	@Test
	void canAttachSignatureToTransaction() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);
		final Transaction transaction = factory.create(transferDescriptor());
		final CryptoTypes.Signature signature = new CryptoTypes.Signature(new byte[CryptoTypes.Signature.SIZE]);

		// Act:
		final String json = NemTransactionFactory.attachSignature(transaction, signature);

		// Assert:
		assertThat(json, startsWith("{\"data\":\""));
		final String expectedData = org.symbol.sdk.utils.Converter
				.uint8ToHex(NemTransactionFactory.toNonVerifiableTransaction(transaction).serialize());
		final String expectedSignature = org.symbol.sdk.utils.Converter.uint8ToHex(new byte[CryptoTypes.Signature.SIZE]);
		assertThat(json, is(equalTo("{\"data\":\"" + expectedData + "\", \"signature\":\"" + expectedSignature + "\"}")));
	}

	@Test
	void canCreateTransactionJsonRepresentation() {
		// Arrange:
		final NemTransactionFactory factory = new NemTransactionFactory(Network.TESTNET);
		final Transaction transaction = factory.create(transferDescriptor());
		final byte[] signatureBytes = new byte[CryptoTypes.Signature.SIZE];
		for (int i = 0; i < signatureBytes.length; ++i)
			signatureBytes[i] = (byte) (i + 0x20);
		final String attached = NemTransactionFactory.attachSignature(transaction, new CryptoTypes.Signature(signatureBytes));

		// Act:
		final String json = NemTransactionFactory.toJson(transaction);

		// Assert: non-verifiable payload plus the attached signature — identical to the attachSignature output
		final String expectedData = org.symbol.sdk.utils.Converter
				.uint8ToHex(NemTransactionFactory.toNonVerifiableTransaction(transaction).serialize());
		final String expectedSignature = org.symbol.sdk.utils.Converter.uint8ToHex(signatureBytes);
		assertThat(json, is(equalTo("{\"data\":\"" + expectedData + "\", \"signature\":\"" + expectedSignature + "\"}")));
		assertThat(json, is(equalTo(attached)));
	}

	// endregion
}
