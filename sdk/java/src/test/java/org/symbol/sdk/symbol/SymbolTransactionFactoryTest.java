package org.symbol.sdk.symbol;

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

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CatbufferType;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.models.*;

/**
 * Tests {@link SymbolTransactionFactory}: rule registration, name lookup, create with address convert and autosort, id auto-generation, and
 * deserialize / attachSignature. Nested classes mirror the JS reference's describe blocks ('lookupTransactionName', 'transaction',
 * 'embedded transaction'); Transaction/EmbeddedTransaction get a Test suffix to avoid colliding with the model types. Like the JS
 * runBasicTransactionFactoryTests / runSymbolTransactionFactoryTests suites, the tests shared by the plain and embedded paths live in the
 * abstract {@link BasicFactoryTest} both nested classes extend.
 */
final class SymbolTransactionFactoryTest {

	private static byte[] patternBytes(final int size, final int seed) {
		final byte[] bytes = new byte[size];
		for (int i = 0; i < bytes.length; ++i)
			bytes[i] = (byte) (seed + i);
		return bytes;
	}

	private static final CryptoTypes.PublicKey TEST_SIGNER_PUBLIC_KEY = new CryptoTypes.PublicKey(
			patternBytes(CryptoTypes.PublicKey.SIZE, 1));

	private static Map<String, Object> transferDescriptor() {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v1");
		descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
		descriptor.put("recipientAddress", new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA"));
		return descriptor;
	}

	private static Map<String, Object> unorderedMosaicsDescriptor() {
		// mosaics deliberately out of order (descending by id)
		final Map<String, Object> descriptor = transferDescriptor();
		descriptor.put("mosaics", List.of(Map.of("mosaicId", 0x72C0212E67A08BCEL, "amount", 1000L),
				Map.of("mosaicId", 0x3A8416DB2D53B0FFL, "amount", 2000L)));
		descriptor.put("message", "Hello, world!".getBytes());
		return descriptor;
	}

	// region constants + rules

	@Test
	void hasRulesWithExpectedHints() {
		// Arrange:
		final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);

		// Act:
		final Set<String> ruleNames = factory.getRuleNames();

		// Assert:
		// every generated pod/enum/flags (Models.FACTORIES) ...
		final String[] expected = {
				"Amount", "BlockDuration", "BlockFeeMultiplier", "Difficulty", "FinalizationEpoch", "FinalizationPoint", "Height",
				"Importance", "ImportanceHeight", "MosaicId", "MosaicNonce", "MosaicRestrictionKey", "NamespaceId", "Timestamp",
				"UnresolvedMosaicId", "Hash512", "Signature", "ProofGamma", "ProofScalar", "ProofVerificationHash",

				"MosaicFlags", "AccountRestrictionFlags",

				"AliasAction", "BlockType", "LinkAction", "LockHashAlgorithm", "MosaicRestrictionType", "MosaicSupplyChangeAction",
				"NamespaceRegistrationType", "NetworkType", "ReceiptType", "TransactionType",

				// ... plus the registered struct parsers ...
				"struct:UnresolvedMosaic",

				// ... the SDK pod overrides (replace the generated entries under the same names) ...
				"UnresolvedAddress", "Address", "Hash256", "PublicKey", "VotingPublicKey",

				// ... and the array parsers.
				"array[UnresolvedMosaicId]", "array[TransactionType]", "array[UnresolvedAddress]", "array[UnresolvedMosaic]"
		};
		assertThat(ruleNames, containsInAnyOrder(expected));
	}

	@Test
	void getRuleNamesReturnsImmutableSnapshot() {
		// Arrange:
		final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);

		// Act:
		final Set<String> ruleNames = factory.getRuleNames();

		// Assert: the returned set is a snapshot — mutating it must fail rather than write through to the factory rules
		assertThrows(UnsupportedOperationException.class, () -> ruleNames.remove("Amount"));
		assertThat(factory.getRuleNames().contains("Amount"), is(true));
	}

	// endregion

	// region lookupTransactionName

	private static void assertLookupName(final TransactionType type, final int version, final String expected) {
		// Act:
		final String actual = SymbolTransactionFactory.lookupTransactionName(type, version);

		// Assert:
		assertThat(actual, is(equalTo(expected)));
	}

	@Nested
	class LookupTransactionName {

		@Test
		void canLookupKnownTransaction() {
			// Act + Assert:
			assertLookupName(TransactionType.TRANSFER, 1, "transfer_transaction_v1");
			assertLookupName(TransactionType.TRANSFER, 2, "transfer_transaction_v2");
			assertLookupName(TransactionType.HASH_LOCK, 1, "hash_lock_transaction_v1");
		}

		@Test
		void cannotLookupUnknownTransaction() {
			// Act + Assert: the enum cannot hold arbitrary unknown values — exercise fromValue directly
			assertThrows(IllegalArgumentException.class, () -> TransactionType.fromValue(123));
		}
	}

	// endregion

	// region shared transaction / embedded transaction suite

	/**
	 * Tests shared by the plain and embedded factory paths; each runs twice, once per {@code @Nested} subclass — the Java analog of the JS
	 * suites parameterized over a testDescriptor. Plain and embedded generated models are unrelated classes, so type-specific fields are
	 * asserted through {@code getField}.
	 */
	abstract class BasicFactoryTest {
		abstract CatbufferType create(SymbolTransactionFactory factory, Map<String, Object> descriptor, boolean autosort);

		abstract CatbufferType deserialize(byte[] payload);

		/** @return Model base-class name used in factory error messages. */
		abstract String modelName();

		final CatbufferType create(final SymbolTransactionFactory factory, final Map<String, Object> descriptor) {
			return create(factory, descriptor, true);
		}

		// region create

		@Test
		void canCreateKnownTransactionFromDescriptor() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);

			// Act:
			final CatbufferType transaction = create(factory, transferDescriptor());

			// Assert:
			assertThat(transaction.getField("type"), is(TransactionType.TRANSFER));
			assertThat(transaction.getField("version"), is(equalTo(1)));
			assertThat(transaction.getField("network"), is(NetworkType.TESTNET));
			assertThat(transaction.getField("signerPublicKey"), is(equalTo(new PublicKey(TEST_SIGNER_PUBLIC_KEY.bytes()))));
		}

		@Test
		void cannotCreateUnknownTransactionFromDescriptor() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final Map<String, Object> descriptor = transferDescriptor();
			descriptor.put("type", "xtransfer_transaction_v1");

			// Act + Assert:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> create(factory, descriptor));
			assertThat(ex.getMessage(), is(equalTo("unknown " + modelName() + " type xtransfer_transaction_v1")));
		}

		@Test
		void canCreateKnownTransactionWithMultipleOverrides() {
			// Arrange: overrides are keyed by rule name and must produce model-typed values (unlike JS, which keys by class)
			final Hash256 fakeHash = new Hash256(patternBytes(CryptoTypes.Hash256.SIZE, 0x40));
			final PublicKey fakeSigner = new PublicKey(patternBytes(CryptoTypes.PublicKey.SIZE, 0x60));
			final Map<String, Function<Object, Object>> typeRuleOverrides = new HashMap<>();
			typeRuleOverrides.put("Hash256", value -> fakeHash);
			typeRuleOverrides.put("BlockDuration", value -> new BlockDuration(654321L));
			typeRuleOverrides.put("PublicKey", value -> fakeSigner);
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET, typeRuleOverrides);

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "hash_lock_transaction_v1");
			descriptor.put("signerPublicKey", "signerName");
			descriptor.put("hash", "not really");
			descriptor.put("duration", "fake duration");
			descriptor.put("mosaic", Map.of("mosaicId", 0x12345678ABCDEFL, "amount", 12345L));

			// Act:
			final CatbufferType transaction = create(factory, descriptor);

			// Assert: the overridden rules produced the fake values; the mosaic struct went through the normal rules
			assertThat(transaction.getField("type"), is(TransactionType.HASH_LOCK));
			assertThat(transaction.getField("version"), is(equalTo(1)));
			assertThat(transaction.getField("network"), is(NetworkType.TESTNET));
			assertThat(transaction.getField("signerPublicKey"), is(equalTo(fakeSigner)));
			assertThat(transaction.getField("hash"), is(equalTo(fakeHash)));
			assertThat(transaction.getField("duration"), is(equalTo(new BlockDuration(654321L))));
			final UnresolvedMosaic mosaic = (UnresolvedMosaic) transaction.getField("mosaic");
			assertThat(mosaic.getMosaicId(), is(equalTo(new UnresolvedMosaicId(0x12345678ABCDEFL))));
			assertThat(mosaic.getAmount(), is(equalTo(new Amount(12345L))));
		}

		// endregion

		// region address type conversion

		@Test
		void canCreateTransactionWithAddress() {
			// Arrange: validates the Address → UnresolvedAddress type-converter.
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final Address a1 = new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA");
			final Address a2 = new Address("DINRYHI6D4QCCIRDEQSSMJZIFEVCWLBNFYXTAMI");
			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "account_address_restriction_transaction_v1");
			descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
			descriptor.put("restrictionAdditions", List.of(a1, a2));

			// Act:
			final CatbufferType transaction = create(factory, descriptor);

			// Assert:
			assertThat(transaction.getField("restrictionAdditions"),
					is(equalTo(List.of(new UnresolvedAddress(a1.bytes()), new UnresolvedAddress(a2.bytes())))));
		}

		// endregion

		// region sorting

		@Test
		void canCreateTransactionWithOutOfOrderArrayWhenAutosortIsEnabled() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);

			// Act:
			final CatbufferType transaction = create(factory, unorderedMosaicsDescriptor());

			// Assert: mosaics were reordered (ascending by mosaicId)
			final List<?> mosaics = (List<?>) transaction.getField("mosaics");
			assertThat(((UnresolvedMosaic) mosaics.get(0)).getMosaicId(), is(equalTo(new UnresolvedMosaicId(0x3A8416DB2D53B0FFL))));
			assertThat(((UnresolvedMosaic) mosaics.get(1)).getMosaicId(), is(equalTo(new UnresolvedMosaicId(0x72C0212E67A08BCEL))));
		}

		@Test
		void cannotCreateTransactionWithOutOfOrderArrayWhenAutosortIsDisabled() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);

			// Act: create without sorting.
			final CatbufferType transaction = create(factory, unorderedMosaicsDescriptor(), false);

			// Assert: mosaics were NOT reordered, and serialization rejects the unsorted array.
			final List<?> mosaics = (List<?>) transaction.getField("mosaics");
			assertThat(((UnresolvedMosaic) mosaics.get(0)).getMosaicId(), is(equalTo(new UnresolvedMosaicId(0x72C0212E67A08BCEL))));
			assertThat(((UnresolvedMosaic) mosaics.get(1)).getMosaicId(), is(equalTo(new UnresolvedMosaicId(0x3A8416DB2D53B0FFL))));
			assertThrows(IllegalArgumentException.class, transaction::serialize);
		}

		// endregion

		// region id autogeneration

		@Test
		void canAutogenerateNamespaceRegistrationRootId() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "namespace_registration_transaction_v1");
			descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
			descriptor.put("registrationType", "root");
			descriptor.put("duration", 123L);
			descriptor.put("name", "roger");

			// Act:
			final CatbufferType transaction = create(factory, descriptor);

			// Assert:
			final long expectedId = IdGenerator.generateNamespaceId("roger");
			assertThat(transaction.getField("id"), is(equalTo(new NamespaceId(expectedId))));
		}

		@Test
		void canAutogenerateNamespaceRegistrationChildId() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final long parentId = IdGenerator.generateNamespaceId("roger");
			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "namespace_registration_transaction_v1");
			descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
			descriptor.put("registrationType", "child");
			descriptor.put("parentId", parentId);
			descriptor.put("name", "charlie");

			// Act:
			final CatbufferType transaction = create(factory, descriptor);

			// Assert:
			final long expectedId = IdGenerator.generateNamespaceId("charlie", parentId);
			assertThat(transaction.getField("id"), is(equalTo(new NamespaceId(expectedId))));
		}

		@Test
		void canAutogenerateMosaicDefinitionId() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "mosaic_definition_transaction_v1");
			descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
			descriptor.put("nonce", 123L);

			// Act:
			final CatbufferType transaction = create(factory, descriptor);

			// Assert:
			final long expectedId = IdGenerator.generateMosaicId(Network.TESTNET.publicKeyToAddress(TEST_SIGNER_PUBLIC_KEY), 123L);
			assertThat(transaction.getField("id"), is(equalTo(new MosaicId(expectedId))));
		}

		// endregion

		// region serialize round-trip

		@Test
		void canDeserializeTransactionFromBuffer() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final CatbufferType original = create(factory, transferDescriptor());
			final byte[] payload = original.serialize();

			// Act:
			final CatbufferType deserialized = deserialize(payload);

			// Assert:
			assertThat(deserialized.getField("type"), is(TransactionType.TRANSFER));
			assertThat(deserialized.serialize(), is(equalTo(payload)));
		}

		// endregion
	}

	// endregion

	// region transaction

	@Nested
	class TransactionTest extends BasicFactoryTest {

		@Override
		CatbufferType create(final SymbolTransactionFactory factory, final Map<String, Object> descriptor, final boolean autosort) {
			return factory.create(descriptor, autosort);
		}

		@Override
		CatbufferType deserialize(final byte[] payload) {
			return SymbolTransactionFactory.deserialize(payload);
		}

		@Override
		String modelName() {
			return "Transaction";
		}

		// region sorting (Java-only)

		@Test
		void canCreateTransferWithImmutableMosaicsList() {
			// Arrange: regression — descriptor with an immutable List (List.of / Map.of) of mosaics
			// must still allow the generated transaction.sort() to sort the mosaics field in-place.
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final Map<String, Object> descriptor = Map.of("type", "transfer_transaction_v1", "signerPublicKey", TEST_SIGNER_PUBLIC_KEY,
					"recipientAddress", new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA"), "mosaics",
					List.of(Map.of("mosaicId", 0x72C0212E67A08BCEL, "amount", 1000L),
							Map.of("mosaicId", 0x3A8416DB2D53B0FFL, "amount", 2000L)),
					"message", "Hello, world!".getBytes());

			// Act: should not throw UnsupportedOperationException from sort()
			final Transaction transaction = factory.create(descriptor);

			// Assert: mosaics are sorted ascending by mosaicId
			final TransferTransactionV1 transfer = (TransferTransactionV1) transaction;
			final List<UnresolvedMosaic> mosaics = transfer.getMosaics();
			assertThat(mosaics.size(), is(2));
			assertThat(mosaics.get(0).getMosaicId().value(), is(equalTo(0x3A8416DB2D53B0FFL)));
			assertThat(mosaics.get(1).getMosaicId().value(), is(equalTo(0x72C0212E67A08BCEL)));
		}

		// endregion

		// region descriptor validation (Java-only)

		@Test
		void childNamespaceRegistrationWithoutParentIdIsRejected() {
			// Arrange: a child registration is parented; a missing parentId must fail fast at create rather than silently deriving
			// a root-style id (which would NPE later during serialize)
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "namespace_registration_transaction_v1");
			descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
			descriptor.put("registrationType", "child");
			descriptor.put("name", "charlie");

			// Act + Assert:
			assertThrows(org.symbol.sdk.InvalidDescriptorException.class, () -> factory.create(descriptor));
		}

		@Test
		void createsAddressAliasWithResolvedAddress() {
			// Arrange: AddressAliasTransaction.address is a resolved Address field (unlike recipientAddress, which is unresolved)
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final byte[] resolvedBytes = new byte[Address.SIZE];
			resolvedBytes[0] = (byte) 0x98; // testnet regular byte (low bit clear) -> resolved, not an alias
			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "address_alias_transaction_v1");
			descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
			descriptor.put("namespaceId", 0x84B3552D375FFA4BL);
			descriptor.put("address", new Address(resolvedBytes));
			descriptor.put("aliasAction", "link");

			// Act:
			final AddressAliasTransactionV1 transaction = (AddressAliasTransactionV1) factory.create(descriptor);

			// Assert: the resolved field holds the model address with the same bytes
			assertThat(transaction.getAddress(), is(equalTo(new org.symbol.sdk.symbol.models.Address(resolvedBytes))));
		}

		@Test
		void createRejectsAliasForResolvedAliasTransactionAddress() {
			// Arrange: a namespace alias is not a valid value for the resolved address field
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final byte[] aliasBytes = new byte[Address.SIZE];
			aliasBytes[0] = (byte) 0x99; // low bit set -> namespace alias
			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "address_alias_transaction_v1");
			descriptor.put("signerPublicKey", TEST_SIGNER_PUBLIC_KEY);
			descriptor.put("namespaceId", 0x84B3552D375FFA4BL);
			descriptor.put("address", new Address(aliasBytes));
			descriptor.put("aliasAction", "link");

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> factory.create(descriptor));
		}

		// endregion

		// region attachSignature + toJson (plain transactions only, as in JS)

		@Test
		void canAttachSignatureToTransaction() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final Transaction transaction = factory.create(transferDescriptor());
			final CryptoTypes.Signature signature = new CryptoTypes.Signature(new byte[CryptoTypes.Signature.SIZE]);

			// Act:
			final String json = SymbolTransactionFactory.attachSignature(transaction, signature);

			// Assert:
			assertThat(json, startsWith("{\"payload\": \""));
			assertThat(json, is(equalTo("{\"payload\": \"" + org.symbol.sdk.utils.Converter.uint8ToHex(transaction.serialize()) + "\"}")));
		}

		@Test
		void canCreateTransactionJsonRepresentation() {
			// Arrange:
			final SymbolTransactionFactory factory = new SymbolTransactionFactory(Network.TESTNET);
			final Transaction transaction = factory.create(transferDescriptor());
			final CryptoTypes.Signature signature = new CryptoTypes.Signature(patternBytes(CryptoTypes.Signature.SIZE, 0x20));
			final String attached = SymbolTransactionFactory.attachSignature(transaction, signature);

			// Act:
			final String json = SymbolTransactionFactory.toJson(transaction);

			// Assert: the payload embeds the serialized transaction, including the attached signature
			assertThat(json, is(equalTo("{\"payload\": \"" + org.symbol.sdk.utils.Converter.uint8ToHex(transaction.serialize()) + "\"}")));
			assertThat(json, is(equalTo(attached)));
		}

		// endregion
	}

	// endregion

	// region embedded transaction

	@Nested
	class EmbeddedTransactionTest extends BasicFactoryTest {

		@Override
		CatbufferType create(final SymbolTransactionFactory factory, final Map<String, Object> descriptor, final boolean autosort) {
			return factory.createEmbedded(descriptor, autosort);
		}

		@Override
		CatbufferType deserialize(final byte[] payload) {
			return SymbolTransactionFactory.deserializeEmbedded(payload);
		}

		@Override
		String modelName() {
			return "EmbeddedTransaction";
		}
	}

	// endregion
}
