package org.symbol.sdk.facade;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.time.Instant;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.Bip32;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.nem.Address;
import org.symbol.sdk.nem.KeyPair;
import org.symbol.sdk.nem.MessageEncoder;
import org.symbol.sdk.nem.NemTransactionFactory;
import org.symbol.sdk.nem.Network;
import org.symbol.sdk.nem.NetworkTimestamp;
import org.symbol.sdk.nem.descriptors.CosignatureV1Descriptor;
import org.symbol.sdk.nem.descriptors.MessageDescriptor;
import org.symbol.sdk.nem.descriptors.MultisigTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.NamespaceRegistrationTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.NemTransactionDescriptor;
import org.symbol.sdk.nem.descriptors.SizePrefixedCosignatureV1Descriptor;
import org.symbol.sdk.nem.descriptors.TransferTransactionV1Descriptor;
import org.symbol.sdk.nem.models.*;

/**
 * Tests {@link NemFacade}: construction, account wrappers, transaction create/sign/verify/hash, BIP32 paths, and typed descriptors.
 */
final class NemFacadeTest {

	private static final CryptoTypes.PrivateKey TEST_PRIVATE_KEY = new CryptoTypes.PrivateKey(
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

	private static final long FEE = 1000L;

	// region real transactions

	private static final CryptoTypes.PrivateKey REAL_SIGNER_PRIVATE_KEY = new CryptoTypes.PrivateKey(
			"EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC");

	private static Transaction createRealTransfer() {
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v1");
		descriptor.put("signerPublicKey", "A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74");
		descriptor.put("fee", 0x186A0L);
		descriptor.put("timestamp", 191205516L);
		descriptor.put("deadline", 191291916L);
		descriptor.put("recipientAddress", "TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW");
		descriptor.put("amount", 5100000L);
		descriptor.put("message", Map.of("messageType", "plain", "message", "blah blah"));
		return facade.transactionFactory.create(descriptor);
	}

	private static Transaction createRealMultisigTransaction() {
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final NonVerifiableTransaction innerTransaction = NemTransactionFactory.toNonVerifiableTransaction(createRealTransfer());

		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "multisig_transaction_v1");
		descriptor.put("signerPublicKey", "A59277D56E9F4FA46854F5EFAAA253B09F8AE69A473565E01FD9E6A738E4AB74");
		descriptor.put("fee", 0x123456L);
		descriptor.put("timestamp", 191205516L);
		descriptor.put("deadline", 191291916L);
		descriptor.put("innerTransaction", innerTransaction);
		return facade.transactionFactory.create(descriptor);
	}

	// endregion

	// region constructors / constants

	@Test
	void hasCorrectBip32Constants() {
		assertThat(NemFacade.BIP32_CURVE_NAME, is(equalTo("ed25519-keccak")));
	}

	@Test
	void canCreateAroundKnownNetworkByName() {
		// Act:
		final NemFacade facade = new NemFacade("testnet");
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v2");
		descriptor.put("signerPublicKey", new CryptoTypes.PublicKey(new byte[CryptoTypes.PublicKey.SIZE]));
		final Transaction transaction = facade.transactionFactory.create(descriptor);

		// Assert:
		assertThat(facade.network, is(sameInstance(Network.TESTNET)));
		assertThat(facade.network.name, is(equalTo("testnet")));
		assertThat(facade.network.identifier, is(equalTo((byte) 0x98)));

		assertThat(transaction.getType().getValue(), is(equalTo(0x0101L)));
		assertThat(transaction.getVersion(), is(equalTo(2)));
	}

	@Test
	void cannotCreateAroundUnknownNetworkByName() {
		assertThrows(RuntimeException.class, () -> new NemFacade("foo"));
	}

	@Test
	void canCreateAroundUnknownNetwork() {
		// Arrange:
		final Network network = new Network("foo", (byte) 0xDE, Instant.EPOCH);

		// Act:
		final NemFacade facade = new NemFacade(network);

		// Assert:
		assertThat(facade.network.name, is(equalTo("foo")));
		assertThat(facade.network.identifier, is(equalTo((byte) 0xDE)));
	}

	@Test
	void canCreateCurrentTimestampForNetworkViaNow() {
		while (true) {
			// Arrange: affinitize test to run so that whole test runs within the context of the same millisecond
			final long startTime = System.currentTimeMillis();
			final NemFacade facade = new NemFacade(Network.TESTNET);

			// Act:
			final NetworkTimestamp nowFromFacade = facade.now();
			final NetworkTimestamp nowFromNetwork = facade.network.fromDatetime(Instant.now());

			final long endTime = System.currentTimeMillis();
			if (startTime != endTime)
				continue;

			// Assert:
			assertThat(nowFromFacade, is(equalTo(nowFromNetwork)));
			assertThat(0L < nowFromFacade.timestamp, is(true));
			break;
		}
	}

	// endregion

	// region account wrappers

	@Test
	void canCreatePublicAccountFromPublicKey() {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final CryptoTypes.PublicKey publicKey = new CryptoTypes.PublicKey(
				"D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0");

		// Act:
		final NemFacade.NemPublicAccount account = facade.createPublicAccount(publicKey);

		// Assert:
		assertThat(account.address, is(equalTo(new Address("TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33"))));
		assertThat(account.publicKey, is(equalTo(publicKey)));
	}

	@Test
	void canCreateAccountFromPrivateKey() {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final CryptoTypes.PublicKey publicKey = new CryptoTypes.PublicKey(
				"D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0");
		final CryptoTypes.PrivateKey privateKey = new CryptoTypes.PrivateKey(
				"ED4C70D78104EB11BCD73EBDC512FEBC8FBCEB36A370C957FF7E266230BB5D57"); // reversed

		// Act:
		final NemFacade.NemAccount account = facade.createAccount(privateKey);

		// Assert:
		assertThat(account.address, is(equalTo(new Address("TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33"))));
		assertThat(account.publicKey, is(equalTo(publicKey)));
		assertThat(account.keyPair.getPublicKey(), is(equalTo(publicKey)));
		assertThat(account.keyPair.getPrivateKey(), is(equalTo(privateKey)));
	}

	@Test
	void canCreateMessageEncoder() {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final NemFacade.NemAccount account = facade.createAccount(REAL_SIGNER_PRIVATE_KEY);

		// Act:
		final MessageEncoder encoder = account.messageEncoder();

		// Assert: message encoder matches the account
		assertThat(encoder.getPublicKey(), is(equalTo(account.publicKey)));
	}

	@Test
	void canSignAndVerifyTransaction() {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final NemFacade.NemAccount account = facade.createAccount(REAL_SIGNER_PRIVATE_KEY);

		final Transaction transaction = createRealTransfer();

		// Sanity:
		assertThat(transaction.getSignature().bytes(), is(equalTo(new byte[CryptoTypes.Signature.SIZE])));

		// Act:
		final CryptoTypes.Signature signature = account.signTransaction(transaction);
		final boolean isVerified = facade.verifyTransaction(transaction, signature);

		// Assert:
		assertThat(isVerified, is(true));
	}

	// endregion

	// region transaction create + sign + verify + hash

	private static Map<String, Object> transferDescriptor(final CryptoTypes.PublicKey signerPublicKey) {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v1");
		descriptor.put("signerPublicKey", signerPublicKey);
		descriptor.put("recipientAddress", new Address("TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C"));
		descriptor.put("amount", 5L);
		return descriptor;
	}

	@Test
	void canCreateTransactionFromDescriptor() {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);

		// Act:
		final Transaction transaction = facade.createTransactionFromDescriptor(transferDescriptor(keyPair.getPublicKey()),
				keyPair.getPublicKey(), FEE, 60L);

		// Assert:
		assertThat(transaction.getFee().value(), is(equalTo(FEE)));
		assertThat(transaction.getDeadline().value() - transaction.getTimestamp().value(), is(equalTo(60L)));
	}

	@Test
	void verifyFailsWithWrongSignature() {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
		final Transaction transaction = facade.createTransactionFromDescriptor(transferDescriptor(keyPair.getPublicKey()),
				keyPair.getPublicKey(), FEE, 60L);

		// Act:
		final CryptoTypes.Signature good = facade.signTransaction(keyPair, transaction);
		final byte[] tampered = good.bytes().clone();
		tampered[0] ^= (byte) 0xFF;

		// Assert:
		assertThat(facade.verifyTransaction(transaction, new CryptoTypes.Signature(tampered)), is(false));
	}

	private static void assertCanHashTransaction(final Supplier<Transaction> transactionFactory, final CryptoTypes.Hash256 expectedHash) {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);

		final Transaction transaction = transactionFactory.get();

		// Act:
		final CryptoTypes.Hash256 hashValue = facade.hashTransaction(transaction);

		// Assert:
		assertThat(hashValue, is(equalTo(expectedHash)));
	}

	private static void assertCanSignTransaction(final Supplier<Transaction> transactionFactory,
			final CryptoTypes.Signature expectedSignature) {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);

		final Transaction transaction = transactionFactory.get();

		// Sanity:
		assertThat(transaction.getSignature().bytes(), is(equalTo(new byte[CryptoTypes.Signature.SIZE])));

		// Act:
		final CryptoTypes.Signature signature = facade.signTransaction(new KeyPair(REAL_SIGNER_PRIVATE_KEY), transaction);

		// Assert:
		assertThat(signature, is(equalTo(expectedSignature)));
	}

	private interface SignOperation {
		CryptoTypes.Signature sign(NemFacade facade, KeyPair keyPair, Transaction transaction);
	}

	private static void assertCanVerifyTransaction(final Supplier<Transaction> transactionFactory, final SignOperation sign) {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);

		final Transaction transaction = transactionFactory.get();

		// Sanity:
		assertThat(transaction.getSignature().bytes(), is(equalTo(new byte[CryptoTypes.Signature.SIZE])));

		// Act:
		final CryptoTypes.Signature signature = sign.sign(facade, new KeyPair(REAL_SIGNER_PRIVATE_KEY), transaction);
		final boolean isVerified = facade.verifyTransaction(transaction, signature);

		// Assert:
		assertThat(isVerified, is(true));
	}

	private static void assertCanVerifySignedTransaction(final Supplier<Transaction> transactionFactory) {
		assertCanVerifyTransaction(transactionFactory, (facade, keyPair, transaction) -> facade.signTransaction(keyPair, transaction));
	}

	private static void assertCanVerifySignedTransactionSigningPayload(final Supplier<Transaction> transactionFactory) {
		assertCanVerifyTransaction(transactionFactory,
				(facade, keyPair, transaction) -> keyPair.sign(facade.extractSigningPayload(transaction)));
	}

	@Test
	void canHashNormalTransaction() {
		assertCanHashTransaction(NemFacadeTest::createRealTransfer,
				new CryptoTypes.Hash256("A7064DB890A4E7329AAB2AE7DCFA5EC76D7E374590C61EC85E03C698DF4EA79D"));
	}

	@Test
	void canSignNormalTransaction() {
		assertCanSignTransaction(NemFacadeTest::createRealTransfer, new CryptoTypes.Signature(
				"23A7B3433D16172E6C8659DB24233C5A8222C589098EA7A8FBBCB19691C67DB13FB2AB7BB215265A3E3D74D32683516B03785BFEB2A2DE6DAC09F5E34A793706"));
	}

	@Test
	void canVerifySignedNormalTransaction() {
		assertCanVerifySignedTransaction(NemFacadeTest::createRealTransfer);
	}

	@Test
	void canVerifySignedNormalTransactionSigningPayload() {
		assertCanVerifySignedTransactionSigningPayload(NemFacadeTest::createRealTransfer);
	}

	@Test
	void canHashMultisigTransaction() {
		assertCanHashTransaction(NemFacadeTest::createRealMultisigTransaction,
				new CryptoTypes.Hash256("B585BC092CDDDCBA535FD6C0DE38F26EB44E6BA638A0BA6DFAD4BAA7E7AAE1B8"));
	}

	@Test
	void canSignMultisigTransaction() {
		assertCanSignTransaction(NemFacadeTest::createRealMultisigTransaction, new CryptoTypes.Signature(
				"E324CCA57275D9752A684E6A089733803423647B8DDF5C1627FC23218CC84287EB7037AD4C6CB8CB37BBC9F5423FA73F431814A008400A756CFFE35F4533EB00"));
	}

	@Test
	void canVerifySignedMultisigTransaction() {
		assertCanVerifySignedTransaction(NemFacadeTest::createRealMultisigTransaction);
	}

	@Test
	void canVerifySignedMultisigTransactionSigningPayload() {
		assertCanVerifySignedTransactionSigningPayload(NemFacadeTest::createRealMultisigTransaction);
	}

	@Test
	void extractSigningPayloadMatchesNonVerifiableSerialization() {
		// Arrange:
		final NemFacade facade = new NemFacade(Network.TESTNET);
		final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
		final Transaction transaction = facade.createTransactionFromDescriptor(transferDescriptor(keyPair.getPublicKey()),
				keyPair.getPublicKey(), FEE, 60L);

		// Act:
		final byte[] payload = facade.extractSigningPayload(transaction);
		final byte[] expected = NemTransactionFactory.toNonVerifiableTransaction(transaction).serialize();

		// Assert:
		assertThat(payload, is(equalTo(expected)));
	}

	// endregion

	// region BIP32

	@Test
	void canConstructProperBip32MainnetPath() {
		// Act:
		final int[] path = new NemFacade(Network.MAINNET).bip32Path(0);

		// Assert:
		assertThat(path, is(equalTo(new int[]{
				44, 43, 0, 0, 0
		})));
	}

	@Test
	void canConstructProperBip32TestnetPath() {
		// Act:
		final int[] path = new NemFacade(Network.TESTNET).bip32Path(7);

		// Assert:
		assertThat(path, is(equalTo(new int[]{
				44, 1, 7, 0, 0
		})));
	}

	@Test
	void bip32NodeToKeyPairReversesPrivateKeyBytes() {
		// Arrange: deterministic 64-byte seed.
		final byte[] seed = new byte[64];
		for (int i = 0; i < seed.length; ++i)
			seed[i] = (byte) i;
		final Bip32.Bip32Node node = new Bip32(NemFacade.BIP32_CURVE_NAME).fromSeed(seed);
		final byte[] reversed = new byte[node.privateKey.bytes().length];
		for (int i = 0; i < reversed.length; ++i)
			reversed[i] = node.privateKey.bytes()[reversed.length - 1 - i];

		// Act:
		final KeyPair derived = NemFacade.bip32NodeToKeyPair(node);
		// Direct reference: KeyPair internally reverses, so passing reversed bytes here yields the same key
		// as passing the BIP32 node through NemFacade.bip32NodeToKeyPair (which reverses too).
		final KeyPair reference = new KeyPair(new CryptoTypes.PrivateKey(reversed));

		// Assert:
		assertThat(derived.getPublicKey().bytes().length, is(CryptoTypes.PublicKey.SIZE));
		assertThat(derived.getPublicKey(), is(equalTo(reference.getPublicKey())));
	}

	private static void assertBip32ChildPublicKeys(final String passphrase, final List<CryptoTypes.PublicKey> expectedChildPublicKeys) {
		// Arrange:
		final String mnemonicSeed = "hamster diagram private dutch cause delay private meat slide toddler razor book "
				+ "happy fancy gospel tennis maple dilemma loan word shrug inflict delay length";

		// Act:
		final Bip32.Bip32Node rootNode = new Bip32(NemFacade.BIP32_CURVE_NAME).fromMnemonic(mnemonicSeed, passphrase);

		final List<CryptoTypes.PublicKey> childPublicKeys = new ArrayList<>();
		for (int i = 0; i < expectedChildPublicKeys.size(); ++i) {
			final Bip32.Bip32Node childNode = rootNode.derivePath(new NemFacade(Network.MAINNET).bip32Path(i));
			childPublicKeys.add(NemFacade.bip32NodeToKeyPair(childNode).getPublicKey());
		}

		// Assert:
		assertThat(childPublicKeys, is(equalTo(expectedChildPublicKeys)));
	}

	@Test
	void canUseBip32DerivationWithoutPassphrase() {
		assertBip32ChildPublicKeys("",
				List.of(new CryptoTypes.PublicKey("6C42BFAD2199CCB5C64E59868CC7A3F2AD29BDDCEB9754157DF136535E6B5EBA"),
						new CryptoTypes.PublicKey("782FF2375F75524106356092B4EE4BA098D28CF6571F1643867B9A11AEF509C6"),
						new CryptoTypes.PublicKey("20EEEFCAE026EBB3C3C51E9AF86A97AA146B34A5463CFE468B37C3CB49682408")));
	}

	@Test
	void canUseBip32DerivationWithPassphrase() {
		assertBip32ChildPublicKeys("TREZOR",
				List.of(new CryptoTypes.PublicKey("3BE4759796DD507D6E410CD8C65121E7E42DAC69699A9058E1F7663A390122CE"),
						new CryptoTypes.PublicKey("6B288C00800EC9FC0C30F35CEAFC2C5EC4066C2BE622822AAC70D67F215E5E6D"),
						new CryptoTypes.PublicKey("1AC159878D327E578C0130767E960C265753CAD5215FC992F1F71C41D00EADA3")));
	}

	// endregion

	// region typed descriptors

	@Nested
	final class TypedDescriptors {
		@Test
		void toMapInjectsTypeAndOmitsNullOptionalFields() {
			// Act: required fields only.
			final Address recipient = new Address("TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C");
			final Amount amount = new Amount(5L);
			final Map<String, Object> map = new TransferTransactionV1Descriptor(recipient, amount).toMap();

			// Assert: discriminator baked in; null optional message field is omitted.
			assertThat(map.get("type"), is(equalTo("transfer_transaction_v1")));
			assertThat(map.get("recipientAddress"), is(sameInstance(recipient)));
			assertThat(map.get("amount"), is(sameInstance(amount)));
			assertThat(map.containsKey("message"), is(false));
		}

		@Test
		void nullOptionalSetterOmitsField() {
			// Act: root-namespace registration — passing null to the optional parentName setter omits it.
			final NamespaceRegistrationTransactionV1Descriptor descriptor = new NamespaceRegistrationTransactionV1Descriptor(
					"TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C", "50000").name("roger").parentName((String) null);

			// Assert: name converted, parentName omitted (not NPE, not a null entry).
			assertThat(descriptor.toMap().get("name"), is(equalTo("roger".getBytes(java.nio.charset.StandardCharsets.UTF_8))));
			assertThat(descriptor.toMap().containsKey("parentName"), is(false));
		}

		@Test
		void multisigWithCosignaturesBuildsFromTypedDescriptor() {
			// Arrange: an inner transfer plus one cosignature, all through typed descriptors.
			final NemFacade facade = new NemFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final Transaction inner = facade.createTransactionFromTypedDescriptor(
					new TransferTransactionV1Descriptor("TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C", "5000000"), keyPair.getPublicKey(), FEE,
					60L);
			final MultisigTransactionV1Descriptor descriptor = new MultisigTransactionV1Descriptor(
					NemTransactionFactory.toNonVerifiableTransaction(inner))
					.cosignatures(new SizePrefixedCosignatureV1Descriptor(
							new CosignatureV1Descriptor("E9B3AEDE9A57C2B8C3D78DB9805D12AB0D983B63CE8F89D8DFE108D0FF08D23C",
									"TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C")));

			// Act: the cosignatures array must flow through the struct/array rules (previously unregistered).
			final Transaction transaction = facade.createTransactionFromTypedDescriptor(descriptor, keyPair.getPublicKey(), FEE, 60L);

			// Assert:
			assertThat(transaction, is(instanceOf(MultisigTransactionV1.class)));
			assertThat(((MultisigTransactionV1) transaction).getCosignatures().size(), is(equalTo(1)));
		}

		@Test
		void canCreateTransactionFromTypedDescriptor() {
			// Arrange:
			final NemFacade facade = new NemFacade(Network.TESTNET);
			final NetworkTimestamp nowTimestamp = facade.now();

			final CryptoTypes.PublicKey signerPublicKey = new CryptoTypes.PublicKey(
					"87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8");
			final NemTransactionDescriptor typedDescriptor = new TransferTransactionV1Descriptor(
					new Address("TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW"), new Amount(1000000L))
					.message(new MessageDescriptor(MessageType.PLAIN).message("hello nem"));

			// Act:
			final TransferTransactionV1 transaction = (TransferTransactionV1) facade.createTransactionFromTypedDescriptor(typedDescriptor,
					signerPublicKey, 100L, 60L * 60L);

			// Assert:
			assertThat(transaction.getType(), is(TransactionType.TRANSFER));
			assertThat(transaction.getVersion(), is(1));
			assertThat(transaction.getNetwork(), is(NetworkType.TESTNET));
			assertThat(transaction.getMessage().get().getMessage(),
					is(equalTo("hello nem".getBytes(java.nio.charset.StandardCharsets.UTF_8))));

			assertThat(transaction.getSignerPublicKey().bytes(), is(equalTo(signerPublicKey.bytes())));
			assertThat(transaction.getFee().value(), is(equalTo(100L)));

			// - check timestamp and deadline are in range (within 10s)
			assertThat(nowTimestamp.timestamp <= transaction.getTimestamp().value(), is(true));
			assertThat(transaction.getTimestamp().value() <= nowTimestamp.timestamp + 10L, is(true));

			final long minRawDeadline = nowTimestamp.timestamp + (60L * 60L);
			assertThat(minRawDeadline <= transaction.getDeadline().value(), is(true));
			assertThat(transaction.getDeadline().value() <= minRawDeadline + 10L, is(true));
		}

		@Test
		void createTransactionFromTypedDescriptorMatchesUntypedPath() {
			// Arrange:
			final NemFacade facade = new NemFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final NemTransactionDescriptor typedDescriptor = new TransferTransactionV1Descriptor(
					new Address("TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C"), new Amount(5L));

			// Act:
			final Transaction typed = facade.createTransactionFromTypedDescriptor(typedDescriptor, keyPair.getPublicKey(), FEE, 60L);
			final Transaction untyped = facade.createTransactionFromDescriptor(typedDescriptor.toMap(), keyPair.getPublicKey(), FEE, 60L);

			// Assert: both paths produce the same transaction shape and fee.
			assertThat(typed.getFee().value(), is(equalTo(FEE)));
			assertThat(typed.size(), is(untyped.size()));
			assertThat(typed.getType(), is(untyped.getType()));
		}
	}

	// endregion

	// region createTransactionFromJson

	@Nested
	final class JsonDescriptors {
		private static final String TRANSFER_JSON = """
				{
					"type": "transfer_transaction_v1",
					"recipientAddress": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
					"amount": 5,
					"message": {"messageType": "plain", "message": "hello nem"}
				}""";

		@Test
		void createTransactionFromJsonMatchesUntypedPath() {
			// Arrange: the equivalent untyped descriptor map.
			final NemFacade facade = new NemFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final Map<String, Object> descriptor = new java.util.LinkedHashMap<>();
			descriptor.put("type", "transfer_transaction_v1");
			descriptor.put("recipientAddress", "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C");
			descriptor.put("amount", 5L);
			final Map<String, Object> message = new java.util.LinkedHashMap<>();
			message.put("messageType", "plain");
			message.put("message", "hello nem");
			descriptor.put("message", message);

			// Act:
			final Transaction fromJson = facade.createTransactionFromJson(TRANSFER_JSON, keyPair.getPublicKey(), FEE, 60L);
			final Transaction fromMap = facade.createTransactionFromDescriptor(descriptor, keyPair.getPublicKey(), FEE, 60L);

			// Assert: identical wire bytes apart from the timestamps, which are computed from "now" per call —
			// normalize them before comparing.
			fromJson.setTimestamp(fromMap.getTimestamp());
			fromJson.setDeadline(fromMap.getDeadline());
			assertThat(fromJson.serialize(), is(equalTo(fromMap.serialize())));
		}

		@Test
		void malformedJsonThrowsInvalidDescriptorException() {
			// Arrange:
			final NemFacade facade = new NemFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);

			// Act + Assert:
			assertThrows(org.symbol.sdk.InvalidDescriptorException.class,
					() -> facade.createTransactionFromJson("{ not json", keyPair.getPublicKey(), FEE, 60L));
		}

		// JSON transaction documents

		@Test
		void canCreateMosaicDefinitionWithStringPropertiesFromJson() {
			// Arrange: the property name/value pairs are auto-encoded from strings to their UTF-8 bytes.
			final NemFacade facade = new NemFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final String json = """
					{
						"type": "mosaic_definition_transaction_v1",
						"mosaicDefinition": {"properties": [{"property": {"name": "divisibility", "value": "2"}}]}
					}""";

			// Act:
			final MosaicDefinitionTransactionV1 transaction = (MosaicDefinitionTransactionV1) facade.createTransactionFromJson(json,
					keyPair.getPublicKey(), FEE, 60L);

			// Assert:
			final java.util.List<SizePrefixedMosaicProperty> properties = transaction.getMosaicDefinition().getProperties();
			assertThat(properties.size(), is(equalTo(1)));
			assertThat(properties.get(0).getProperty().getName(),
					is(equalTo("divisibility".getBytes(java.nio.charset.StandardCharsets.UTF_8))));
			assertThat(properties.get(0).getProperty().getValue(), is(equalTo("2".getBytes(java.nio.charset.StandardCharsets.UTF_8))));
		}

		@Test
		void canCreateNamespaceRegistrationFromJson() {
			// Arrange: the namespace name string is auto-encoded to its UTF-8 bytes; the rental fee sink parses from a
			// base32 address string.
			final NemFacade facade = new NemFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final String json = """
					{
						"type": "namespace_registration_transaction_v1",
						"rentalFeeSink": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
						"rentalFee": 50000000,
						"name": "roger"
					}""";

			// Act:
			final NamespaceRegistrationTransactionV1 transaction = (NamespaceRegistrationTransactionV1) facade
					.createTransactionFromJson(json, keyPair.getPublicKey(), FEE, 60L);

			// Assert:
			assertThat(transaction.getRentalFee(), is(equalTo(new Amount(50000000L))));
			assertThat(transaction.getName(), is(equalTo("roger".getBytes(java.nio.charset.StandardCharsets.UTF_8))));
		}
	}

	// endregion
}
