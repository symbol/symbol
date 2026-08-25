package org.symbol.sdk.facade;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.greaterThan;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.time.Instant;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.Bip32;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.MessageEncoder;
import org.symbol.sdk.symbol.Network;
import org.symbol.sdk.symbol.NetworkTimestamp;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.AggregateCompleteTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.SymbolTransactionDescriptor;
import org.symbol.sdk.symbol.descriptors.TransferTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.UnresolvedMosaicDescriptor;
import org.symbol.sdk.symbol.models.*;
import org.symbol.sdk.utils.Transforms;

/**
 * Tests {@link SymbolFacade}: construction, account wrappers, transaction create/sign/verify/hash, cosigning, aggregate hashing, BIP32
 * paths, and typed descriptors.
 */
final class SymbolFacadeTest {

	private static final CryptoTypes.PrivateKey TEST_PRIVATE_KEY = new CryptoTypes.PrivateKey(
			"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

	// region real transactions

	private static final String REAL_SIGNER_HEX = "87DA603E7BE5656C45692D5FC7F6D0EF8F24BB7A5C10ED5FDA8C5CFBC49FCBC8";

	private static final CryptoTypes.PrivateKey REAL_SIGNER_PRIVATE_KEY = new CryptoTypes.PrivateKey(
			"EDB671EB741BD676969D8A035271D1EE5E75DF33278083D877F23615EB839FEC");

	private static Transaction createRealTransfer(final SymbolFacade facade) {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v1");
		descriptor.put("signerPublicKey", REAL_SIGNER_HEX);
		descriptor.put("fee", 1000000L);
		descriptor.put("deadline", 41998024783L);
		descriptor.put("recipientAddress", "TD4PJKW5JP3CNHA47VDFIM25RCWTWRGT45HMPSA");
		descriptor.put("mosaics", List.of(Map.of("mosaicId", 0x2CF403E85507F39EL, "amount", 1000000L)));
		return facade.transactionFactory.create(descriptor);
	}

	private static AggregateCompleteTransactionV1 createRealAggregate(final SymbolFacade facade) {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "aggregate_complete_transaction_v1");
		descriptor.put("signerPublicKey", REAL_SIGNER_HEX);
		descriptor.put("fee", 2000000L);
		descriptor.put("deadline", 42238390163L);
		descriptor.put("transactionsHash", "71554638F578358B1D3FC4369AC625DB491AD5E5D4424D6DBED9FFC7411A37FE");
		final AggregateCompleteTransactionV1 aggregate = (AggregateCompleteTransactionV1) facade.transactionFactory.create(descriptor);

		final Map<String, Object> transferDescriptor = new LinkedHashMap<>();
		transferDescriptor.put("type", "transfer_transaction_v1");
		transferDescriptor.put("signerPublicKey", REAL_SIGNER_HEX);
		transferDescriptor.put("recipientAddress", "TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA");
		transferDescriptor.put("mosaics", List.of(Map.of("mosaicId", 0x2CF403E85507F39EL, "amount", 1000000L)));
		aggregate.getTransactions().add(facade.transactionFactory.createEmbedded(transferDescriptor));
		return aggregate;
	}

	private static List<EmbeddedTransaction> createRealEmbeddedTransactions(final SymbolFacade facade) {
		final Map<String, Object> transfer = new LinkedHashMap<>();
		transfer.put("type", "transfer_transaction_v1");
		transfer.put("signerPublicKey", REAL_SIGNER_HEX);
		transfer.put("recipientAddress", "TCIDK4CGCHGVZHLNTOKJ32MFEZWMFBCWUJIAXCA");
		transfer.put("mosaics", List.of(Map.of("mosaicId", 0x2CF403E85507F39EL, "amount", 1000000L)));

		final Map<String, Object> secretProof = new LinkedHashMap<>();
		secretProof.put("type", "secret_proof_transaction_v1");
		secretProof.put("signerPublicKey", REAL_SIGNER_HEX);
		secretProof.put("recipientAddress", "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y");
		secretProof.put("secret", "BE254D2744329BBE20F9CF6DA61043B4CEF8C2BC000000000000000000000000");
		secretProof.put("hashAlgorithm", "hash_256");
		secretProof.put("proof", "41FB");

		final Map<String, Object> addressAlias = new LinkedHashMap<>();
		addressAlias.put("type", "address_alias_transaction_v1");
		addressAlias.put("signerPublicKey", REAL_SIGNER_HEX);
		addressAlias.put("namespaceId", 0xA95F1F8A96159516L);
		addressAlias.put("address", "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y");
		addressAlias.put("aliasAction", "link");

		final List<EmbeddedTransaction> transactions = new ArrayList<>();
		for (final Map<String, Object> embeddedDescriptor : List.of(transfer, secretProof, addressAlias))
			transactions.add(facade.transactionFactory.createEmbedded(embeddedDescriptor));

		return transactions;
	}

	private static Transaction createRealAggregateSwap(final SymbolFacade facade) {
		final Map<String, Object> firstTransfer = new LinkedHashMap<>();
		firstTransfer.put("type", "transfer_transaction_v1");
		firstTransfer.put("signerPublicKey", "29856F43A5C4CBDE42F2FAC775A6F915E9E5638CF458E9352E7B410B662473A3");
		firstTransfer.put("recipientAddress", "TBEZ3VKFBMKQSW7APBVL5NWNBEU7RR466PRRTDQ");
		firstTransfer.put("mosaics", List.of(Map.of("mosaicId", 0xE74B99BA41F4AFEEL, "amount", 20000000L)));

		final Map<String, Object> secondTransfer = new LinkedHashMap<>();
		secondTransfer.put("type", "transfer_transaction_v1");
		secondTransfer.put("signerPublicKey", "4C94E8B0A1DAB8573BCB6632E676F742E0D320FC8102F20FB7FB13BCAE9A9F60");
		secondTransfer.put("recipientAddress", "TDFR3Q3H5W4OPOSHALVDY3RF4ZQNH44LIUIHYTQ");
		secondTransfer.put("mosaics", List.of(Map.of("mosaicId", 0x798A29F48E927C83L, "amount", 100L)));

		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "aggregate_complete_transaction_v1");
		descriptor.put("signerPublicKey", "4C94E8B0A1DAB8573BCB6632E676F742E0D320FC8102F20FB7FB13BCAE9A9F60");
		descriptor.put("fee", 36000L);
		descriptor.put("deadline", 26443750218L);
		descriptor.put("transactionsHash", "641CB7E431F1D44094A43E1CE8265E6BD1DF1C3B0B64797CDDAA0A375FCD3C08");
		descriptor.put("transactions",
				List.of(facade.transactionFactory.createEmbedded(firstTransfer), facade.transactionFactory.createEmbedded(secondTransfer)));
		return facade.transactionFactory.create(descriptor);
	}

	private static void attachSignature(final Transaction transaction, final CryptoTypes.Signature signature) {
		SymbolTransactionFactory.attachSignature(transaction, signature);
	}

	// endregion

	// region constructors / constants

	@Test
	void hasCorrectBip32Constants() {
		assertThat(SymbolFacade.BIP32_CURVE_NAME, is(equalTo("ed25519")));
	}

	@Test
	void canCreateAroundKnownNetworkByName() {
		// Act:
		final SymbolFacade facade = new SymbolFacade("testnet");
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v1");
		descriptor.put("signerPublicKey", new CryptoTypes.PublicKey(new byte[CryptoTypes.PublicKey.SIZE]));
		final Transaction transaction = facade.transactionFactory.create(descriptor);

		// Assert:
		assertThat(facade.network, is(sameInstance(Network.TESTNET)));
		assertThat(facade.network.name, is(equalTo("testnet")));
		assertThat(facade.network.identifier, is(equalTo((byte) 0x98)));

		assertThat(transaction.getType().getValue(), is(equalTo(0x4154)));
		assertThat(transaction.getVersion(), is(equalTo(1)));
	}

	@Test
	void cannotCreateAroundUnknownNetworkByName() {
		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> new SymbolFacade("foo"));

		// Assert:
		assertThat(ex.getMessage(), containsString("no network found with name 'foo'"));
	}

	@Test
	void canCreateAroundUnknownNetwork() {
		// Arrange:
		final Network network = new Network("foo", (byte) 0xDE, Instant.EPOCH, new CryptoTypes.Hash256(new byte[32]));

		// Act:
		final SymbolFacade facade = new SymbolFacade(network);

		// Assert: (the JS test also creates a transaction; the Java NetworkType enum rejects the unknown identifier 0xDE,
		// so transaction creation on an unknown network is unsupported — an accepted enum-strictness divergence)
		assertThat(facade.network.name, is(equalTo("foo")));
		assertThat(facade.network.identifier, is(equalTo((byte) 0xDE)));
	}

	@Test
	void canCreateCurrentTimestampForNetworkViaNow() {
		while (true) {
			// Arrange: affinitize test to run so that whole test runs within the context of the same millisecond
			final long startTime = System.currentTimeMillis();
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);

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

	// region cosign helpers

	private interface AggregateSignOperation {
		CryptoTypes.Signature sign(SymbolFacade facade, CryptoTypes.PrivateKey privateKey, Transaction transaction);
	}

	private static final AggregateSignOperation SIGN_VIA_KEY_PAIR = (facade, privateKey, transaction) -> facade
			.signTransaction(new KeyPair(privateKey), transaction);

	private static final AggregateSignOperation SIGN_VIA_ACCOUNT = (facade, privateKey, transaction) -> facade.createAccount(privateKey)
			.signTransaction(transaction);

	private interface AttachedCosignOperation {
		Cosignature cosign(SymbolFacade facade, CryptoTypes.PrivateKey privateKey, Transaction transaction);
	}

	private interface DetachedCosignOperation {
		DetachedCosignature cosign(SymbolFacade facade, CryptoTypes.PrivateKey privateKey, Transaction transaction);
	}

	private static final CryptoTypes.PrivateKey COSIGNER_PRIVATE_KEY = new CryptoTypes.PrivateKey(
			"BE7B98F835A896136ADDAF04220F28CB4925D24F0675A21421BF213C180BEF86");

	private static Transaction arrangeSignedAggregateSwap(final SymbolFacade facade, final AggregateSignOperation signOperation) {
		final CryptoTypes.PrivateKey signerPrivateKey = new CryptoTypes.PrivateKey(
				"F4BC233E183E8CEA08D0A604A3DC67FF3261D1E6EBF84D233488BC53D89C50B7");
		final Transaction transaction = createRealAggregateSwap(facade);
		attachSignature(transaction, signOperation.sign(facade, signerPrivateKey, transaction));
		return transaction;
	}

	private static void assertCosignatureCommonFields(final long version, final byte[] signerPublicKeyBytes, final byte[] signatureBytes) {
		assertThat(version, is(equalTo(0L)));
		assertThat(signerPublicKeyBytes,
				is(equalTo(new CryptoTypes.PublicKey("29856F43A5C4CBDE42F2FAC775A6F915E9E5638CF458E9352E7B410B662473A3").bytes())));
		assertThat(signatureBytes, is(equalTo(new CryptoTypes.Signature(
				"204BD2C4F86B66313E5C5F817FD650B108826D53EDEFC8BDFF936E4D6AA07E385F819CF0BF22D14D4AA2011AD07BC0FE6023E2CB48DC5D82A6A1FF1348FA3E0B")
				.bytes())));
	}

	private static void assertCanCosignTransactionAttached(final AggregateSignOperation signOperation,
			final AttachedCosignOperation operation) {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
		final Transaction transaction = arrangeSignedAggregateSwap(facade, signOperation);

		// Act:
		final Cosignature cosignature = operation.cosign(facade, COSIGNER_PRIVATE_KEY, transaction);

		// Assert: check common fields; the cosignature should be suitable for attaching to an aggregate
		assertCosignatureCommonFields(cosignature.getVersion(), cosignature.getSignerPublicKey().bytes(),
				cosignature.getSignature().bytes());
		assertThat(cosignature.size(), is(equalTo(104)));
	}

	private static void assertCanCosignTransactionDetached(final AggregateSignOperation signOperation,
			final DetachedCosignOperation operation) {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
		final Transaction transaction = arrangeSignedAggregateSwap(facade, signOperation);

		// Act:
		final DetachedCosignature cosignature = operation.cosign(facade, COSIGNER_PRIVATE_KEY, transaction);

		// Assert: check common fields; the cosignature should be detached
		assertCosignatureCommonFields(cosignature.getVersion(), cosignature.getSignerPublicKey().bytes(),
				cosignature.getSignature().bytes());
		assertThat(cosignature.size(), is(equalTo(136)));
		assertThat(cosignature.getParentHash().bytes(),
				is(equalTo(new CryptoTypes.Hash256("214DFF47469D462E1D9A03232C2582C7E44DE026A287F98529CC74DE9BD69641").bytes())));
	}

	// endregion

	// region account wrappers

	@Nested
	final class AccountWrappers {
		@Test
		void canCreatePublicAccountFromPublicKey() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final CryptoTypes.PublicKey publicKey = new CryptoTypes.PublicKey(
					"E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD");

			// Act:
			final SymbolFacade.SymbolPublicAccount account = facade.createPublicAccount(publicKey);

			// Assert:
			assertThat(account.address(), is(equalTo(new Address("TABDOFVM2QYIMVNQII6UJWU7Y66GZI4LQTMN4PI"))));
			assertThat(account.publicKey(), is(equalTo(publicKey)));
		}

		@Test
		void canCreateAccountFromPrivateKey() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final CryptoTypes.PublicKey publicKey = new CryptoTypes.PublicKey(
					"E29C5934F44482E7A9F50725C8681DE6CA63F49E5562DB7E5BC9EABA31356BAD");
			final CryptoTypes.PrivateKey privateKey = new CryptoTypes.PrivateKey(
					"E88283CE35FE74C89FFCB2D8BFA0A2CF6108BDC0D07606DEE34D161C30AC2F1E");

			// Act:
			final SymbolFacade.SymbolAccount account = facade.createAccount(privateKey);

			// Assert:
			assertThat(account.address(), is(equalTo(new Address("TABDOFVM2QYIMVNQII6UJWU7Y66GZI4LQTMN4PI"))));
			assertThat(account.publicKey(), is(equalTo(publicKey)));
			assertThat(account.keyPair().getPublicKey(), is(equalTo(publicKey)));
			assertThat(account.keyPair().getPrivateKey(), is(equalTo(privateKey)));
		}

		@Test
		void canCreateMessageEncoder() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final SymbolFacade.SymbolAccount account = facade.createAccount(REAL_SIGNER_PRIVATE_KEY);

			// Act:
			final MessageEncoder encoder = account.messageEncoder();

			// Assert: message encoder matches the account
			assertThat(encoder.getPublicKey(), is(equalTo(account.publicKey())));
		}

		@Test
		void canSignTransaction() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final SymbolFacade.SymbolAccount account = facade.createAccount(REAL_SIGNER_PRIVATE_KEY);

			final Transaction transaction = createRealTransfer(facade);

			// Sanity:
			assertThat(transaction.getSignature().bytes(), is(equalTo(new byte[CryptoTypes.Signature.SIZE])));

			// Act:
			final CryptoTypes.Signature signature = account.signTransaction(transaction);
			final boolean isVerified = facade.verifyTransaction(transaction, signature);

			// Assert:
			assertThat(isVerified, is(true));
		}

		@Test
		void canCosignTransactionAsAttachedCosignature() {
			assertCanCosignTransactionAttached(SIGN_VIA_ACCOUNT,
					(facade, privateKey, transaction) -> facade.createAccount(privateKey).cosignTransaction(transaction));
		}

		@Test
		void canCosignTransactionAsDetachedCosignature() {
			assertCanCosignTransactionDetached(SIGN_VIA_ACCOUNT,
					(facade, privateKey, transaction) -> facade.createAccount(privateKey).cosignTransactionDetached(transaction));
		}

		@Test
		void canCosignTransactionHashAsAttachedCosignature() {
			assertCanCosignTransactionAttached(SIGN_VIA_ACCOUNT, (facade, privateKey, transaction) -> facade.createAccount(privateKey)
					.cosignTransactionHash(facade.hashTransaction(transaction)));
		}

		@Test
		void canCosignTransactionHashAsDetachedCosignature() {
			assertCanCosignTransactionDetached(SIGN_VIA_ACCOUNT, (facade, privateKey, transaction) -> facade.createAccount(privateKey)
					.cosignTransactionHashDetached(facade.hashTransaction(transaction)));
		}
	}

	// endregion

	// region transaction create + sign + verify + hash

	private static Map<String, Object> transferDescriptor(final CryptoTypes.PublicKey signerPublicKey) {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer_transaction_v1");
		descriptor.put("signerPublicKey", signerPublicKey);
		descriptor.put("recipientAddress", new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA"));
		return descriptor;
	}

	private Transaction newTransfer(final SymbolFacade facade, final KeyPair keyPair) {
		return facade.createTransactionFromDescriptor(transferDescriptor(keyPair.getPublicKey()), keyPair.getPublicKey(), 100L, 60L);
	}

	private static void assertTwoCosignatureFeeDelta(final Transaction txNoCos, final Transaction txTwoCos) {
		// extra 2 cosignatures multiplied by 100 fee multiplier.
		final long expectedDelta = 2L * new Cosignature().size() * 100L;
		assertThat(txTwoCos.getFee().value() - txNoCos.getFee().value(), is(equalTo(expectedDelta)));
	}

	@Test
	void cosignatureCountIncreasesFee() {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
		final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);

		// Act:
		final Transaction txNoCos = facade.createTransactionFromDescriptor(transferDescriptor(keyPair.getPublicKey()),
				keyPair.getPublicKey(), 100L, 60L, 0);
		final Transaction txTwoCos = facade.createTransactionFromDescriptor(transferDescriptor(keyPair.getPublicKey()),
				keyPair.getPublicKey(), 100L, 60L, 2);

		// Assert:
		assertTwoCosignatureFeeDelta(txNoCos, txTwoCos);
	}

	@Test
	void canCreateEmbeddedTransactionFromDescriptor() {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
		final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);

		// Act:
		final EmbeddedTransaction transaction = facade.createEmbeddedTransactionFromDescriptor(transferDescriptor(keyPair.getPublicKey()),
				keyPair.getPublicKey());

		// Assert:
		assertThat(transaction.getType(), is(TransactionType.TRANSFER));
	}

	@Test
	void extractSigningPayloadStartsWithGenerationHashSeed() {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
		final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
		final Transaction transaction = newTransfer(facade, keyPair);

		// Act:
		final byte[] payload = facade.extractSigningPayload(transaction);

		// Assert: signing payload == generationHashSeed | <rest>
		final byte[] seed = facade.network.generationHashSeed.bytes();
		assertThat(Arrays.copyOfRange(payload, 0, seed.length), is(equalTo(seed)));
		assertThat(payload.length, is(greaterThan(seed.length)));
	}

	private static void assertCanHashTransaction(final java.util.function.Function<SymbolFacade, Transaction> transactionFactory,
			final CryptoTypes.Hash256 expectedHash) {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);

		final Transaction transaction = transactionFactory.apply(facade);
		attachSignature(transaction, facade.signTransaction(new KeyPair(REAL_SIGNER_PRIVATE_KEY), transaction));

		// Act:
		final CryptoTypes.Hash256 hashValue = facade.hashTransaction(transaction);

		// Assert:
		assertThat(hashValue, is(equalTo(expectedHash)));
	}

	@Test
	void canHashTransaction() {
		assertCanHashTransaction(SymbolFacadeTest::createRealTransfer,
				new CryptoTypes.Hash256("86E006F0D400A781A15D0293DFC15897078351A2F7731D49A865A63C2010DE44"));
	}

	@Test
	void canHashAggregateTransaction() {
		assertCanHashTransaction(SymbolFacadeTest::createRealAggregate,
				new CryptoTypes.Hash256("D074716D62F4CDF1CE219D7E0580DC2C030102E216ECE2037FA28A3BC5726BD0"));
	}

	private static void assertCanSignTransaction(final java.util.function.Function<SymbolFacade, Transaction> transactionFactory,
			final CryptoTypes.Signature expectedSignature) {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);

		final Transaction transaction = transactionFactory.apply(facade);

		// Sanity:
		assertThat(transaction.getSignature().bytes(), is(equalTo(new byte[CryptoTypes.Signature.SIZE])));

		// Act:
		final CryptoTypes.Signature signature = facade.signTransaction(new KeyPair(REAL_SIGNER_PRIVATE_KEY), transaction);

		// Assert:
		assertThat(signature, is(equalTo(expectedSignature)));
	}

	@Test
	void canSignTransaction() {
		assertCanSignTransaction(SymbolFacadeTest::createRealTransfer, new CryptoTypes.Signature(
				"24A3788AFD0223083D47ED14F17A2499A7939CD62C4B3288C40CF2736B13F4048486680DD574C9F7DB56F453464058CB22349ACBFAECAE16A31EF0725FFF6104"));
	}

	@Test
	void canSignAggregateTransaction() {
		assertCanSignTransaction(SymbolFacadeTest::createRealAggregate, new CryptoTypes.Signature(
				"40C5C9F0BAF74E64877982C411D0D16665E18D463B66204081D846564FC6CAE13F1F75C688CBD2D34263DA166537A90B4F371C1B38DDF00414AB0F5D78C3CD0F"));
	}

	private interface SignOperation {
		CryptoTypes.Signature sign(SymbolFacade facade, KeyPair keyPair, Transaction transaction);
	}

	private static void assertCanVerifyTransaction(final java.util.function.Function<SymbolFacade, Transaction> transactionFactory,
			final SignOperation sign) {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);

		final Transaction transaction = transactionFactory.apply(facade);

		// Sanity:
		assertThat(transaction.getSignature().bytes(), is(equalTo(new byte[CryptoTypes.Signature.SIZE])));

		// Act:
		final CryptoTypes.Signature signature = sign.sign(facade, new KeyPair(REAL_SIGNER_PRIVATE_KEY), transaction);
		final boolean isVerified = facade.verifyTransaction(transaction, signature);

		// Assert:
		assertThat(isVerified, is(true));
	}

	private static void assertCanVerifySignedTransaction(final java.util.function.Function<SymbolFacade, Transaction> transactionFactory) {
		assertCanVerifyTransaction(transactionFactory, (facade, keyPair, transaction) -> facade.signTransaction(keyPair, transaction));
	}

	@Test
	void canVerifySignedTransaction() {
		assertCanVerifySignedTransaction(SymbolFacadeTest::createRealTransfer);
	}

	@Test
	void canVerifySignedAggregateTransaction() {
		assertCanVerifySignedTransaction(SymbolFacadeTest::createRealAggregate);
	}

	private static void assertCanVerifySignedTransactionSigningPayload(
			final java.util.function.Function<SymbolFacade, Transaction> transactionFactory) {
		assertCanVerifyTransaction(transactionFactory,
				(facade, keyPair, transaction) -> keyPair.sign(facade.extractSigningPayload(transaction)));
	}

	@Test
	void canVerifySignedTransactionSigningPayload() {
		assertCanVerifySignedTransactionSigningPayload(SymbolFacadeTest::createRealTransfer);
	}

	@Test
	void canVerifySignedAggregateTransactionSigningPayload() {
		assertCanVerifySignedTransactionSigningPayload(SymbolFacadeTest::createRealAggregate);
	}

	// endregion

	// region conditional aggregate data buffers

	@Test
	void respectsConditionalAggregateDataBuffer() {
		// Arrange: V1/V2 hash 52 aggregate body bytes, V3+ hash 56 (the additional payloadSize bytes); mimic
		// SymbolFacade.hashTransaction to implicitly check the version-dependent aggregate data size.
		final int transactionHeaderSize = 108;
		for (final int[] versionAndSize : new int[][]{
				{
						1, 52
				}, {
						2, 52
				}, {
						3, 56
				}, {
						4, 56
				}
		}) {
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);

			final Transaction transaction = createRealAggregate(facade);
			transaction.setVersion(versionAndSize[0]);
			attachSignature(transaction, facade.signTransaction(new KeyPair(REAL_SIGNER_PRIVATE_KEY), transaction));

			// Act:
			final CryptoTypes.Hash256 hashValue = facade.hashTransaction(transaction);

			// Assert:
			final byte[] serializedTransaction = transaction.serialize();
			final CryptoTypes.Hash256 expectedHash = new CryptoTypes.Hash256(Transforms.sha3_256(transaction.getSignature().bytes(),
					transaction.getSignerPublicKey().bytes(), facade.network.generationHashSeed.bytes(),
					Arrays.copyOfRange(serializedTransaction, transactionHeaderSize, transactionHeaderSize + versionAndSize[1])));
			assertThat("version " + versionAndSize[0], hashValue, is(equalTo(expectedHash)));
		}
	}

	// endregion

	// region cosignTransaction

	@Nested
	final class CanCosignTransaction {
		@Test
		void asAttachedCosignature() {
			assertCanCosignTransactionAttached(SIGN_VIA_KEY_PAIR,
					(facade, privateKey, transaction) -> facade.cosignTransaction(new KeyPair(privateKey), transaction));
		}

		@Test
		void asDetachedCosignature() {
			assertCanCosignTransactionDetached(SIGN_VIA_KEY_PAIR,
					(facade, privateKey, transaction) -> facade.cosignTransactionDetached(new KeyPair(privateKey), transaction));
		}

		@Test
		void hashAsAttachedCosignature() {
			assertCanCosignTransactionAttached(SIGN_VIA_KEY_PAIR, (facade, privateKey, transaction) -> SymbolFacade
					.cosignTransactionHash(new KeyPair(privateKey), facade.hashTransaction(transaction)));
		}

		@Test
		void hashAsDetachedCosignature() {
			assertCanCosignTransactionDetached(SIGN_VIA_KEY_PAIR, (facade, privateKey, transaction) -> SymbolFacade
					.cosignTransactionHashDetached(new KeyPair(privateKey), facade.hashTransaction(transaction)));
		}
	}

	// endregion

	// region misc

	@Test
	void canHashEmbeddedTransactions() {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
		final AggregateCompleteTransactionV1 transaction = createRealAggregate(facade);

		// Act:
		final CryptoTypes.Hash256 hashValue = SymbolFacade.hashEmbeddedTransactions(transaction.getTransactions());

		// Assert:
		assertThat(hashValue.bytes(), is(equalTo(transaction.getTransactionsHash().bytes())));
	}

	@Test
	void canHashEmbeddedTransactionsMultiple() {
		// Arrange:
		final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
		final List<EmbeddedTransaction> transactions = createRealEmbeddedTransactions(facade);

		// Act:
		final CryptoTypes.Hash256 hashValue = SymbolFacade.hashEmbeddedTransactions(transactions);

		// Assert:
		assertThat(hashValue, is(equalTo(new CryptoTypes.Hash256("5C78999F21EA75B880100E1B4C76166B9C320869F67C00D28F9F8F754D7831C9"))));
	}

	@Test
	void canConstructProperBip32MainnetPath() {
		// Act:
		final int[] path = new SymbolFacade(Network.MAINNET).bip32Path(2);

		// Assert:
		assertThat(path, is(equalTo(new int[]{
				44, 4343, 2, 0, 0
		})));
	}

	@Test
	void canConstructProperBip32TestnetPath() {
		// Act:
		final int[] path = new SymbolFacade(Network.TESTNET).bip32Path(2);

		// Assert:
		assertThat(path, is(equalTo(new int[]{
				44, 1, 2, 0, 0
		})));
	}

	private static void assertBip32ChildPublicKeys(final String passphrase, final List<CryptoTypes.PublicKey> expectedChildPublicKeys) {
		// Arrange:
		final String mnemonicSeed = "hamster diagram private dutch cause delay private meat slide toddler razor book "
				+ "happy fancy gospel tennis maple dilemma loan word shrug inflict delay length";

		// Act:
		final Bip32.Bip32Node rootNode = new Bip32(SymbolFacade.BIP32_CURVE_NAME).fromMnemonic(mnemonicSeed, passphrase);

		final List<CryptoTypes.PublicKey> childPublicKeys = new ArrayList<>();
		for (int i = 0; i < expectedChildPublicKeys.size(); ++i) {
			final Bip32.Bip32Node childNode = rootNode.derivePath(new SymbolFacade(Network.MAINNET).bip32Path(i));
			childPublicKeys.add(SymbolFacade.bip32NodeToKeyPair(childNode).getPublicKey());
		}

		// Assert:
		assertThat(childPublicKeys, is(equalTo(expectedChildPublicKeys)));
	}

	@Test
	void canUseBip32DerivationWithoutPassphrase() {
		assertBip32ChildPublicKeys("",
				List.of(new CryptoTypes.PublicKey("E9CFE9F59CB4393E61B2F42769D9084A644B16883C32C2823E7DF9A3AF83C121"),
						new CryptoTypes.PublicKey("0DE8C3235271E4C9ACF5482F7DFEC1E5C4B20FFC71548703EACF593153F116F9"),
						new CryptoTypes.PublicKey("259866A68A00C325713342232056333D60710E223FC920566B3248B266E899D5")));
	}

	@Test
	void canUseBip32DerivationWithPassphrase() {
		assertBip32ChildPublicKeys("TREZOR",
				List.of(new CryptoTypes.PublicKey("47F4D39D36D11C07735D7BE99220696AAEE7B3EE161D61422220DFE3FF120B15"),
						new CryptoTypes.PublicKey("4BA67E87E8C14F3EB82B3677EA959B56A9D7355705019CED1FCF6C76104E628C"),
						new CryptoTypes.PublicKey("8115D75C13C2D25E7FA3009D03D63F1F32601CDCCA9244D5FDAC74BCF3E892E3")));
	}

	// endregion

	// region typed descriptors

	@Nested
	final class TypedDescriptors {
		@Test
		void toMapInjectsTypeAndOmitsNullOptionalFields() {
			// Act: only the required recipientAddress is supplied.
			final Address recipient = new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA");
			final Map<String, Object> map = new TransferTransactionV1Descriptor(recipient, (List<UnresolvedMosaicDescriptor>) null, (byte[]) null).toMap();

			// Assert: discriminator baked in; null optional fields are not present.
			assertThat(map.get("type"), is(equalTo("transfer_transaction_v1")));
			assertThat(map.get("recipientAddress"), is(sameInstance(recipient)));
			assertThat(map.containsKey("mosaics"), is(false));
			assertThat(map.containsKey("message"), is(false));
		}

		@Test
		void toMapUnwrapsNestedDescriptorArrays() {
			// Arrange: an array of nested descriptors must be stored as a list of their maps.
			final Address recipient = new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA");
			final UnresolvedMosaicDescriptor mosaic = new UnresolvedMosaicDescriptor(new UnresolvedMosaicId(0x72C0212E67A08BCEL),
					new Amount(1000L));
			final byte[] message = "Hello, world!".getBytes();

			// Act:
			final Map<String, Object> map = new TransferTransactionV1Descriptor(recipient, List.of(mosaic), message).toMap();

			// Assert: mosaics is a list of raw descriptor maps (not descriptor objects).
			assertThat(map.get("message"), is(sameInstance(message)));
			final Object mosaics = map.get("mosaics");
			assertThat(mosaics, is(instanceOf(List.class)));
			final List<?> mosaicList = (List<?>) mosaics;
			assertThat(mosaicList.size(), is(1));
			assertThat(mosaicList.get(0), is(instanceOf(Map.class)));
			assertThat(((Map<?, ?>) mosaicList.get(0)).get("amount"), is(instanceOf(Amount.class)));
		}

		@Test
		void createTransactionFromTypedDescriptorMatchesUntypedPath() {
			// Arrange: a typed descriptor and the equivalent untyped descriptor map.
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final Address recipient = new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA");
			final SymbolTransactionDescriptor typedDescriptor = new TransferTransactionV1Descriptor(recipient, (List<UnresolvedMosaicDescriptor>) null,
					(byte[]) null);

			// Act:
			final Transaction typed = facade.createTransactionFromTypedDescriptor(typedDescriptor, keyPair.getPublicKey(), 100L, 60L);
			final Transaction untyped = facade.createTransactionFromDescriptor(typedDescriptor.toMap(), keyPair.getPublicKey(), 100L, 60L);

			// Assert: identical wire bytes apart from the deadline, which is computed from "now" per call —
			// normalize it before comparing.
			typed.setDeadline(untyped.getDeadline());
			assertThat(typed.serialize(), is(equalTo(untyped.serialize())));
		}

		@Test
		void cosignatureCountIsForwardedToUntypedPath() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final SymbolTransactionDescriptor typedDescriptor = new TransferTransactionV1Descriptor(
					new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA"), (List<UnresolvedMosaicDescriptor>) null, (byte[]) null);

			// Act:
			final Transaction txNoCos = facade.createTransactionFromTypedDescriptor(typedDescriptor, keyPair.getPublicKey(), 100L, 60L, 0);
			final Transaction txTwoCos = facade.createTransactionFromTypedDescriptor(typedDescriptor, keyPair.getPublicKey(), 100L, 60L, 2);

			// Assert:
			assertTwoCosignatureFeeDelta(txNoCos, txTwoCos);
		}

		@Test
		void canCreateTransactionFromTypedDescriptor() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final NetworkTimestamp nowTimestamp = facade.now();

			final CryptoTypes.PublicKey signerPublicKey = new CryptoTypes.PublicKey(REAL_SIGNER_HEX);
			final SymbolTransactionDescriptor typedDescriptor = new TransferTransactionV1Descriptor(
					new Address("TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I"),
					List.of(new UnresolvedMosaicDescriptor(new UnresolvedMosaicId(0x7CDF3B117A3C40CCL), new Amount(1000000L))),
					"hello symbol".getBytes(java.nio.charset.StandardCharsets.UTF_8));

			// Act:
			final TransferTransactionV1 transaction = (TransferTransactionV1) facade.createTransactionFromTypedDescriptor(typedDescriptor,
					signerPublicKey, 100L, 60L * 60L);

			// Assert:
			assertThat(transaction.getType(), is(TransactionType.TRANSFER));
			assertThat(transaction.getVersion(), is(1));
			assertThat(transaction.getNetwork(), is(NetworkType.TESTNET));
			assertThat(transaction.getMessage(), is(equalTo("hello symbol".getBytes(java.nio.charset.StandardCharsets.UTF_8))));

			assertThat(transaction.getSignerPublicKey().bytes(), is(equalTo(signerPublicKey.bytes())));
			assertThat(transaction.getFee().value(), is(equalTo((long) transaction.size() * 100L)));

			// - check deadline is in range (within 10s)
			final long minRawDeadline = nowTimestamp.timestamp + (60L * 60L * 1000L);
			assertThat(minRawDeadline <= transaction.getDeadline().value(), is(true));
			assertThat(transaction.getDeadline().value() <= minRawDeadline + 10000L, is(true));
		}

		private void assertAggregateSizeCalculation(final int descriptorCosignatureCount, final int reservedCosignatureCount,
				final int expectedCosignatureCount) {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);

			final CryptoTypes.PublicKey signerPublicKey = new CryptoTypes.PublicKey(REAL_SIGNER_HEX);
			final List<Cosignature> cosignatures = new ArrayList<>();
			for (int i = 0; i < descriptorCosignatureCount; ++i)
				cosignatures.add(new Cosignature());

			// a zero count passes null so the cosignatures key stays absent, like the old unset state
			final AggregateCompleteTransactionV1Descriptor typedDescriptor = new AggregateCompleteTransactionV1Descriptor(
					new CryptoTypes.Hash256("157D3C15A677030DBD106C0C16556E305F3796B66F684715E0C18FC178DC8026"), List.of(),
					0 == descriptorCosignatureCount ? null : cosignatures);

			// Act:
			final Transaction transaction = facade.createTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey, 100L, 60L * 60L,
					reservedCosignatureCount);

			// Assert: check size and fee
			assertThat(transaction.size(), is(equalTo(168 + 104 * descriptorCosignatureCount)));
			assertThat(transaction.getFee().value(), is(equalTo((long) (168 + 104 * expectedCosignatureCount) * 100L)));
		}

		@Test
		void canCreateAggregateTransactionFromTypedDescriptorWithExplicitCosignatures() {
			assertAggregateSizeCalculation(3, 0, 3);
		}

		@Test
		void canCreateAggregateTransactionFromTypedDescriptorWithImplicitCosignatures() {
			assertAggregateSizeCalculation(0, 4, 4);
		}

		@Test
		void canCreateAggregateTransactionFromTypedDescriptorWithBothExplicitAndImplicitCosignatures() {
			// Assert: maximum of two values should be used in fee calculation
			assertAggregateSizeCalculation(3, 4, 4);
			assertAggregateSizeCalculation(4, 3, 4);
			assertAggregateSizeCalculation(4, 4, 4);
		}

		@Test
		void canCreateEmbeddedTransactionFromTypedDescriptor() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);

			final CryptoTypes.PublicKey signerPublicKey = new CryptoTypes.PublicKey(REAL_SIGNER_HEX);
			final SymbolTransactionDescriptor typedDescriptor = new TransferTransactionV1Descriptor(
					new Address("TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I"),
					List.of(new UnresolvedMosaicDescriptor(new UnresolvedMosaicId(0x7CDF3B117A3C40CCL), new Amount(1000000L))),
					"hello symbol".getBytes(java.nio.charset.StandardCharsets.UTF_8));

			// Act:
			final EmbeddedTransferTransactionV1 transaction = (EmbeddedTransferTransactionV1) facade
					.createEmbeddedTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey);

			// Assert:
			assertThat(transaction.getType(), is(TransactionType.TRANSFER));
			assertThat(transaction.getVersion(), is(1));
			assertThat(transaction.getNetwork(), is(NetworkType.TESTNET));
			assertThat(transaction.getMessage(), is(equalTo("hello symbol".getBytes(java.nio.charset.StandardCharsets.UTF_8))));

			assertThat(transaction.getSignerPublicKey().bytes(), is(equalTo(signerPublicKey.bytes())));
		}
	}

	// endregion

	// region createTransactionFromJson

	@Nested
	final class JsonDescriptors {
		private static final String TRANSFER_JSON = FacadeTestData.SYMBOL_TRANSFER_JSON;

		private Map<String, Object> equivalentMap() {
			final Map<String, Object> descriptor = new java.util.LinkedHashMap<>();
			descriptor.put("type", "transfer_transaction_v1");
			descriptor.put("recipientAddress", "AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA");
			final Map<String, Object> mosaic = new java.util.LinkedHashMap<>();
			mosaic.put("mosaicId", 8589934593L);
			mosaic.put("amount", 1000000L);
			descriptor.put("mosaics", List.of(mosaic));
			descriptor.put("message", "hello symbol");
			return descriptor;
		}

		@Test
		void createTransactionFromJsonMatchesUntypedPath() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);

			// Act:
			final Transaction fromJson = facade.createTransactionFromJson(TRANSFER_JSON, keyPair.getPublicKey(), 100L, 60L);
			final Transaction fromMap = facade.createTransactionFromDescriptor(equivalentMap(), keyPair.getPublicKey(), 100L, 60L);

			// Assert: identical wire bytes apart from the deadline, which is computed from "now" per call —
			// normalize it before comparing.
			fromJson.setDeadline(fromMap.getDeadline());
			assertThat(fromJson.serialize(), is(equalTo(fromMap.serialize())));
		}

		@Test
		void cosignatureCountIsForwardedToUntypedPath() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);

			// Act:
			final Transaction txNoCos = facade.createTransactionFromJson(TRANSFER_JSON, keyPair.getPublicKey(), 100L, 60L, 0);
			final Transaction txTwoCos = facade.createTransactionFromJson(TRANSFER_JSON, keyPair.getPublicKey(), 100L, 60L, 2);

			// Assert:
			assertTwoCosignatureFeeDelta(txNoCos, txTwoCos);
		}

		@Test
		void createEmbeddedTransactionFromJsonMatchesUntypedPath() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);

			// Act:
			final EmbeddedTransaction fromJson = facade.createEmbeddedTransactionFromJson(TRANSFER_JSON, keyPair.getPublicKey());
			final EmbeddedTransaction fromMap = facade.createEmbeddedTransactionFromDescriptor(equivalentMap(), keyPair.getPublicKey());

			// Assert:
			assertThat(fromJson.serialize(), is(equalTo(fromMap.serialize())));
		}

		@Test
		void malformedJsonThrowsInvalidDescriptorException() {
			// Arrange:
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);

			// Act + Assert:
			assertThrows(org.symbol.sdk.InvalidDescriptorException.class,
					() -> facade.createTransactionFromJson("{ not json", keyPair.getPublicKey(), 100L, 60L));
		}

		// JSON transfer documents

		@Test
		void canCreateAccountAddressRestrictionFromJson() {
			// Arrange: flags parse from a space-separated name string; the additions from base32 address strings.
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final String json = """
					{
						"type": "account_address_restriction_transaction_v1",
						"restrictionFlags": "address outgoing block",
						"restrictionAdditions": ["AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA", "DINRYHI6D4QCCIRDEQSSMJZIFEVCWLBNFYXTAMI"]
					}""";

			// Act:
			final AccountAddressRestrictionTransactionV1 transaction = (AccountAddressRestrictionTransactionV1) facade
					.createTransactionFromJson(json, keyPair.getPublicKey(), 100L, 60L);

			// Assert: address(1) | outgoing(16384) | block(32768)
			assertThat(transaction.getRestrictionFlags(), is(equalTo(new AccountRestrictionFlags(49153))));
			assertThat(transaction.getRestrictionAdditions(),
					is(equalTo(List.of(new UnresolvedAddress(new Address("AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA").bytes()),
							new UnresolvedAddress(new Address("DINRYHI6D4QCCIRDEQSSMJZIFEVCWLBNFYXTAMI").bytes())))));
		}

		@Test
		void canCreateNamespaceRegistrationFromJson() {
			// Arrange: the namespace name string is auto-encoded to its UTF-8 bytes.
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final String json = """
					{
						"type": "namespace_registration_transaction_v1",
						"registrationType": "root",
						"duration": 12345,
						"name": "roger"
					}""";

			// Act:
			final NamespaceRegistrationTransactionV1 transaction = (NamespaceRegistrationTransactionV1) facade
					.createTransactionFromJson(json, keyPair.getPublicKey(), 100L, 60L);

			// Assert:
			assertThat(transaction.getRegistrationType(), is(NamespaceRegistrationType.ROOT));
			assertThat(transaction.getDuration().map(BlockDuration::value), is(equalTo(java.util.Optional.of(12345L))));
			assertThat(transaction.getName(), is(equalTo("roger".getBytes(java.nio.charset.StandardCharsets.UTF_8))));
		}

		@Test
		void canCreateAccountMetadataWithBareNumberScopedKeyFromJson() {
			// Arrange: a scoped metadata key always has its high bit set (Metadata.generateKey), so as a bare JSON number
			// it exceeds Long.MAX and deserializes to BigInteger — which the u64 coercion must accept
			final SymbolFacade facade = new SymbolFacade(Network.TESTNET);
			final KeyPair keyPair = new KeyPair(TEST_PRIVATE_KEY);
			final String json = """
					{
						"type": "account_metadata_transaction_v1",
						"targetAddress": "AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA",
						"scopedMetadataKey": 13835058055282163712,
						"value": "sample value"
					}""";

			// Act:
			final AccountMetadataTransactionV1 transaction = (AccountMetadataTransactionV1) facade.createTransactionFromJson(json,
					keyPair.getPublicKey(), 100L, 60L);

			// Assert: 13835058055282163712 == 0xC000000000000000
			assertThat(transaction.getScopedMetadataKey(), is(equalTo(0xC000000000000000L)));
		}
	}

	// endregion
}
