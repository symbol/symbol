package org.symbol.sdk.facade;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.greaterThanOrEqualTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.lessThanOrEqualTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.function.Function;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.CatbufferType;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.NetworkTimestamp;
import org.symbol.sdk.TypedDescriptor;
import org.symbol.sdk.Verifier;

/**
 * Tests {@link FacadeFactory} and its {@link BlockchainFacade} adapters. (Java-only) — the runtime-selected facade abstraction has no
 * JS/Python counterpart; those SDKs choose a facade class dynamically. The shared {@link ChainTests} suite runs against both blockchains
 * through {@link Nested} subclasses that supply the chain hooks; every adapter behavior is compared to the concrete facade (or a pinned
 * value) so adapting cannot change output unnoticed.
 */
final class FacadeFactoryTest {
	private static final CryptoTypes.PrivateKey TEST_PRIVATE_KEY = new CryptoTypes.PrivateKey(
			"575DBB3062267EFF57C970A336EBBC8FBCFE12C5BD3ED7BC11EB0481D7704CED");
	private static final byte[] VERIFY_MESSAGE = {
			1, 2, 3
	};
	private static final long TRANSACTION_DEADLINE_SECONDS = 3600L;
	private static final long DEADLINE_SCHEDULING_DRIFT_SECONDS = 10L;
	private static final long NOW_TOLERANCE_SECONDS = 5L;

	// region shared helpers

	private static BlockchainFacade<?, ?, ?> createAdapter(final String blockchain) {
		return FacadeFactory.create(blockchain, "testnet");
	}

	private static <K extends KeyPair> K createFacadeKeyPair(final Function<CryptoTypes.PrivateKey, K> factory) {
		return factory.apply(TEST_PRIVATE_KEY);
	}

	private static <T extends CatbufferType> T asModel(final CatbufferType transaction, final Class<T> modelClass) {
		return modelClass.cast(transaction);
	}

	private static CryptoTypes.Signature tamperSignature(final CryptoTypes.Signature signature) {
		final byte[] bytes = signature.bytes().clone();
		bytes[0] ^= (byte) 0xFF;
		return new CryptoTypes.Signature(bytes);
	}

	/** A transaction built through an adapter, plus one signature for it. */
	private record SignedTransaction(CatbufferType transaction, CryptoTypes.Signature signature) {
	}

	private static <T extends CatbufferType, D extends TypedDescriptor, K extends KeyPair> SignedTransaction signWithAdapter(
			final BlockchainFacade<T, D, K> adapter, final String json, final long fee, final long deadlineSeconds) {
		final Account<T, K> account = adapter.createAccount(TEST_PRIVATE_KEY);
		final T transaction = adapter.createTransactionFromJson(json, account.publicKey(), fee, deadlineSeconds);
		return new SignedTransaction(transaction, adapter.signTransaction(account.keyPair(), transaction));
	}

	private static <T extends CatbufferType, D extends TypedDescriptor, K extends KeyPair> SignedTransaction signWithAccount(
			final BlockchainFacade<T, D, K> adapter, final String json, final long fee, final long deadlineSeconds) {
		final Account<T, K> account = adapter.createAccount(TEST_PRIVATE_KEY);
		final T transaction = adapter.createTransactionFromJson(json, account.publicKey(), fee, deadlineSeconds);
		return new SignedTransaction(transaction, account.signTransaction(transaction));
	}

	/** A transaction built through an adapter, plus that adapter's hash and signing payload for it. */
	private record BuiltTransaction(CatbufferType transaction, CryptoTypes.Hash256 hash, byte[] signingPayload) {
	}

	// build the transaction and take its hash / signing payload through the adapter (the wildcard capture ties them together)
	private static <T extends CatbufferType, D extends TypedDescriptor, K extends KeyPair> BuiltTransaction buildThroughAdapter(
			final BlockchainFacade<T, D, K> adapter, final String json, final CryptoTypes.PublicKey signerPublicKey, final long fee,
			final long deadlineSeconds) {
		final T transaction = adapter.createTransactionFromJson(json, signerPublicKey, fee, deadlineSeconds);
		return new BuiltTransaction(transaction, adapter.hashTransaction(transaction), adapter.extractSigningPayload(transaction));
	}

	/** A transaction signed through an adapter, both probe signatures, and the adapter's verify output for each. */
	private record VerifyProbe(CatbufferType transaction, CryptoTypes.Signature signature, CryptoTypes.Signature tamperedSignature,
			boolean adapterVerifiesIntact, boolean adapterVerifiesTampered) {
	}

	private static <T extends CatbufferType, D extends TypedDescriptor, K extends KeyPair> VerifyProbe verifyThroughAdapter(
			final BlockchainFacade<T, D, K> adapter, final String json, final long fee, final long deadlineSeconds) {
		final Account<T, K> account = adapter.createAccount(TEST_PRIVATE_KEY);
		final T transaction = adapter.createTransactionFromJson(json, account.publicKey(), fee, deadlineSeconds);
		final CryptoTypes.Signature signature = adapter.signTransaction(account.keyPair(), transaction);
		final CryptoTypes.Signature tamperedSignature = tamperSignature(signature);
		return new VerifyProbe(transaction, signature, tamperedSignature, adapter.verifyTransaction(transaction, signature),
				adapter.verifyTransaction(transaction, tamperedSignature));
	}

	// endregion

	// region factory

	@Test
	void createIsCaseInsensitive() {
		// Act:
		final BlockchainFacade<?, ?, ?> adapter = FacadeFactory.create("Symbol", "mainnet");

		// Assert:
		assertThat(adapter.bip32CurveName(), is(equalTo(SymbolFacade.BIP32_CURVE_NAME)));
	}

	@Test
	void cannotCreateNullBlockchain() {
		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> FacadeFactory.create(null, "testnet"));

		// Assert:
		assertThat(ex.getMessage(), is(equalTo("blockchain name is required")));
	}

	@Test
	void cannotCreateUnknownBlockchain() {
		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> FacadeFactory.create("foo", "testnet"));

		// Assert:
		assertThat(ex.getMessage(), is(equalTo("unknown blockchain: foo")));
	}

	// endregion

	/** The chain-agnostic adapter suite; each blockchain's nested class supplies the hooks and its chain-specific fee/deadline tests. */
	private abstract class ChainTests {
		// region chain hooks

		abstract String blockchainName();

		abstract String expectedBip32CurveName();

		abstract String transferJson();

		abstract long fee();

		abstract long nowTolerance();

		abstract int[] expectedMainnetBip32Path();

		abstract int[] expectedTestnetBip32Path();

		abstract Class<? extends KeyPair> keyPairClass();

		abstract KeyPair createFacadeKeyPair();

		abstract Account<?, ?> createFacadeAccount();

		abstract PublicAccount createFacadePublicAccount(CryptoTypes.PublicKey publicKey);

		abstract long facadeNow();

		abstract CryptoTypes.Signature facadeSign(CatbufferType transaction);

		abstract boolean facadeVerify(CatbufferType transaction, CryptoTypes.Signature signature);

		abstract CryptoTypes.Hash256 facadeHash(CatbufferType transaction);

		abstract byte[] facadeSigningPayload(CatbufferType transaction);

		abstract Verifier createChainVerifier(CryptoTypes.PublicKey publicKey);

		BlockchainFacade<?, ?, ?> createAdapter() {
			return FacadeFactoryTest.createAdapter(blockchainName());
		}

		// endregion

		@Test
		void canCreateFacade() {
			// Act:
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();

			// Assert:
			assertThat(adapter.bip32CurveName(), is(equalTo(expectedBip32CurveName())));
			assertThat(adapter.network().name, is(equalTo("testnet")));
		}

		@Test
		void accountMatchesFacadeAccount() {
			// Arrange:
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();

			// Act:
			final Account<?, ?> account = adapter.createAccount(TEST_PRIVATE_KEY);
			final Account<?, ?> facadeAccount = createFacadeAccount();

			// Assert: the adapter account mirrors the facade account
			assertThat(account.publicKey(), is(equalTo(facadeAccount.publicKey())));
			assertThat(account.address(), is(equalTo(facadeAccount.address())));
			assertThat(account.keyPair().getPrivateKey(), is(equalTo(facadeAccount.keyPair().getPrivateKey())));
		}

		@Test
		void publicAccountMatchesFacadePublicAccount() {
			// Arrange:
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();
			final CryptoTypes.PublicKey publicKey = createFacadeKeyPair().getPublicKey();

			// Act:
			final PublicAccount publicAccount = adapter.createPublicAccount(publicKey);
			final PublicAccount facadePublicAccount = createFacadePublicAccount(publicKey);

			// Assert: the adapter public account mirrors the facade public account
			assertThat(publicAccount.publicKey(), is(equalTo(facadePublicAccount.publicKey())));
			assertThat(publicAccount.address(), is(equalTo(facadePublicAccount.address())));
		}

		@Test
		void createAddressParsesFacadeAddress() {
			// Arrange:
			final Account<?, ?> facadeAccount = createFacadeAccount();
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();

			// Act:
			final Object address = adapter.createAddress(facadeAccount.address().toString());

			// Assert: the adapter parses to the same address the facade derived
			assertThat(address, is(equalTo(facadeAccount.address())));
		}

		@Test
		void createKeyPairIsChainTyped() {
			// Act:
			final KeyPair keyPair = createAdapter().createKeyPair(TEST_PRIVATE_KEY);

			// Assert: chain-typed, and deriving the same public key as the concrete SDK key pair
			assertThat(keyPair, is(instanceOf(keyPairClass())));
			assertThat(keyPair.getPublicKey(), is(equalTo(createFacadeKeyPair().getPublicKey())));
		}

		@Test
		void bip32PathDependsOnNetwork() {
			// Act: same blockchain, different networks — only the coin-type element should change with the network
			final int[] mainnetPath = FacadeFactory.create(blockchainName(), "mainnet").bip32Path(2);
			final int[] testnetPath = FacadeFactory.create(blockchainName(), "testnet").bip32Path(2);

			// Assert:
			assertThat(mainnetPath, is(equalTo(expectedMainnetBip32Path())));
			assertThat(testnetPath, is(equalTo(expectedTestnetBip32Path())));
		}

		@Test
		void nowMatchesFacadeNetworkTime() {
			// Arrange:
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();

			// Act: both derive network time from the same epoch, called microseconds apart
			final long adapterNow = adapter.now().timestamp;
			final long facadeNow = facadeNow();

			// Assert: the adapter delegates now() to the facade (allow a small wall-clock window)
			assertThat(Math.abs(adapterNow - facadeNow), is(lessThanOrEqualTo(nowTolerance())));
		}

		@Test
		void adapterSignatureMatchesFacade() {
			// Act:
			final SignedTransaction signed = signWithAdapter(createAdapter(), transferJson(), fee(), TRANSACTION_DEADLINE_SECONDS);

			// Assert: the adapter delegates signTransaction to the facade (ed25519 is deterministic, so the signatures are identical)
			assertThat(signed.signature(), is(equalTo(facadeSign(signed.transaction()))));
		}

		@Test
		void accountSignatureMatchesFacade() {
			// Act:
			final SignedTransaction signed = signWithAccount(createAdapter(), transferJson(), fee(), TRANSACTION_DEADLINE_SECONDS);

			// Assert:
			assertThat(signed.signature(), is(equalTo(facadeSign(signed.transaction()))));
		}

		@Test
		void adapterVerifyMatchesFacade() {
			// Act:
			final VerifyProbe probe = verifyThroughAdapter(createAdapter(), transferJson(), fee(), TRANSACTION_DEADLINE_SECONDS);

			// Assert: adapting must not change verify output — same result as the facade for an intact and a tampered signature
			assertThat(probe.adapterVerifiesIntact(), is(equalTo(facadeVerify(probe.transaction(), probe.signature()))));
			assertThat(probe.adapterVerifiesTampered(), is(equalTo(facadeVerify(probe.transaction(), probe.tamperedSignature()))));
		}

		@Test
		void verifierFromAdapterMatchesChainVerifier() {
			// Arrange: the facades expose no verifier factory, so the chain Verifier class is the ground truth
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();
			final Account<?, ?> account = adapter.createAccount(TEST_PRIVATE_KEY);
			final CryptoTypes.Signature signature = account.keyPair().sign(VERIFY_MESSAGE);
			final CryptoTypes.Signature tamperedSignature = tamperSignature(signature);
			final Verifier chainVerifier = createChainVerifier(account.publicKey());

			// Act:
			final Verifier verifier = adapter.createVerifier(account.publicKey());

			// Assert: adapting must not change verify output — same result as the chain verifier for both probe signatures
			assertThat(verifier.verify(VERIFY_MESSAGE, signature), is(equalTo(chainVerifier.verify(VERIFY_MESSAGE, signature))));
			assertThat(verifier.verify(VERIFY_MESSAGE, tamperedSignature),
					is(equalTo(chainVerifier.verify(VERIFY_MESSAGE, tamperedSignature))));
		}

		@Test
		void adapterHashMatchesFacade() {
			// Act:
			final BuiltTransaction built = buildThroughAdapter(createAdapter(), transferJson(), createFacadeKeyPair().getPublicKey(), fee(),
					TRANSACTION_DEADLINE_SECONDS);

			// Assert: the adapter delegates hashTransaction to the facade (identical hash for the same transaction)
			assertThat(built.hash(), is(equalTo(facadeHash(built.transaction()))));
		}

		@Test
		void adapterSigningPayloadMatchesFacade() {
			// Act:
			final BuiltTransaction built = buildThroughAdapter(createAdapter(), transferJson(), createFacadeKeyPair().getPublicKey(), fee(),
					TRANSACTION_DEADLINE_SECONDS);

			// Assert: the adapter delegates extractSigningPayload to the facade
			assertThat(built.signingPayload(), is(equalTo(facadeSigningPayload(built.transaction()))));
		}
	}

	@Nested
	final class Symbol extends ChainTests {
		private static final String TRANSFER_JSON = FacadeTestData.SYMBOL_TRANSFER_JSON;
		private static final int[] MAINNET_BIP32_PATH = {
				44, 4343, 2, 0, 0
		};
		private static final int[] TESTNET_BIP32_PATH = {
				44, 1, 2, 0, 0
		};
		// Symbol timestamps are milliseconds
		private static final long NOW_TOLERANCE_MS = NOW_TOLERANCE_SECONDS * 1000L;
		private static final long FEE_MULTIPLIER = 100L;
		private static final long DEADLINE_SCHEDULING_DRIFT_MS = DEADLINE_SCHEDULING_DRIFT_SECONDS * 1000L;

		private final SymbolFacade facade = new SymbolFacade("testnet");

		private org.symbol.sdk.symbol.models.Transaction asModel(final CatbufferType transaction) {
			return FacadeFactoryTest.asModel(transaction, org.symbol.sdk.symbol.models.Transaction.class);
		}

		@Override
		String blockchainName() {
			return "symbol";
		}

		@Override
		String expectedBip32CurveName() {
			return SymbolFacade.BIP32_CURVE_NAME;
		}

		@Override
		String transferJson() {
			return TRANSFER_JSON;
		}

		@Override
		long fee() {
			return FEE_MULTIPLIER;
		}

		@Override
		long nowTolerance() {
			return NOW_TOLERANCE_MS;
		}

		@Override
		int[] expectedMainnetBip32Path() {
			return MAINNET_BIP32_PATH;
		}

		@Override
		int[] expectedTestnetBip32Path() {
			return TESTNET_BIP32_PATH;
		}

		@Override
		Class<? extends KeyPair> keyPairClass() {
			return org.symbol.sdk.symbol.KeyPair.class;
		}

		@Override
		KeyPair createFacadeKeyPair() {
			return FacadeFactoryTest.createFacadeKeyPair(org.symbol.sdk.symbol.KeyPair::new);
		}

		@Override
		Account<?, ?> createFacadeAccount() {
			return facade.createAccount(TEST_PRIVATE_KEY);
		}

		@Override
		PublicAccount createFacadePublicAccount(final CryptoTypes.PublicKey publicKey) {
			return facade.createPublicAccount(publicKey);
		}

		@Override
		long facadeNow() {
			return facade.now().timestamp;
		}

		@Override
		CryptoTypes.Signature facadeSign(final CatbufferType transaction) {
			return facade.signTransaction(new org.symbol.sdk.symbol.KeyPair(TEST_PRIVATE_KEY), asModel(transaction));
		}

		@Override
		boolean facadeVerify(final CatbufferType transaction, final CryptoTypes.Signature signature) {
			return facade.verifyTransaction(asModel(transaction), signature);
		}

		@Override
		CryptoTypes.Hash256 facadeHash(final CatbufferType transaction) {
			return facade.hashTransaction(asModel(transaction));
		}

		@Override
		byte[] facadeSigningPayload(final CatbufferType transaction) {
			return facade.extractSigningPayload(asModel(transaction));
		}

		@Override
		Verifier createChainVerifier(final CryptoTypes.PublicKey publicKey) {
			return new org.symbol.sdk.symbol.Verifier(publicKey);
		}

		@Test
		void adapterAppliesFeeMultiplier() {
			// Arrange:
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();
			final CryptoTypes.PublicKey signerPublicKey = adapter.createKeyPair(TEST_PRIVATE_KEY).getPublicKey();

			// Act:
			final org.symbol.sdk.symbol.models.Transaction transaction = asModel(
					adapter.createTransactionFromJson(TRANSFER_JSON, signerPublicKey, FEE_MULTIPLIER, TRANSACTION_DEADLINE_SECONDS));

			// Assert: Symbol fee is feeMultiplier * size, proving the fee-multiplier argument landed as the multiplier (not the
			// deadline)
			assertThat(transaction.getFee().value(), is(equalTo((long) transaction.size() * FEE_MULTIPLIER)));
		}

		@Test
		void adapterAppliesDeadline() {
			// Arrange:
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();
			final CryptoTypes.PublicKey signerPublicKey = adapter.createKeyPair(TEST_PRIVATE_KEY).getPublicKey();
			final NetworkTimestamp.Base now = adapter.now();

			// Act:
			final org.symbol.sdk.symbol.models.Transaction transaction = asModel(
					adapter.createTransactionFromJson(TRANSFER_JSON, signerPublicKey, FEE_MULTIPLIER, TRANSACTION_DEADLINE_SECONDS));

			// Assert: deadline is deadlineSeconds past now (allow a small wall-clock window)
			final long minDeadline = now.timestamp + TRANSACTION_DEADLINE_SECONDS * 1000L;
			assertThat(transaction.getDeadline().value(), is(greaterThanOrEqualTo(minDeadline)));
			assertThat(transaction.getDeadline().value(), is(lessThanOrEqualTo(minDeadline + DEADLINE_SCHEDULING_DRIFT_MS)));
		}
	}

	@Nested
	final class Nem extends ChainTests {
		private static final String TRANSFER_JSON = FacadeTestData.NEM_TRANSFER_JSON;
		private static final int[] MAINNET_BIP32_PATH = {
				44, 43, 2, 0, 0
		};
		private static final int[] TESTNET_BIP32_PATH = {
				44, 1, 2, 0, 0
		};
		private static final long ABSOLUTE_FEE = 100000L;

		private final NemFacade facade = new NemFacade("testnet");

		private org.symbol.sdk.nem.models.Transaction asModel(final CatbufferType transaction) {
			return FacadeFactoryTest.asModel(transaction, org.symbol.sdk.nem.models.Transaction.class);
		}

		@Override
		String blockchainName() {
			return "nem";
		}

		@Override
		String expectedBip32CurveName() {
			return NemFacade.BIP32_CURVE_NAME;
		}

		@Override
		String transferJson() {
			return TRANSFER_JSON;
		}

		@Override
		long fee() {
			return ABSOLUTE_FEE;
		}

		@Override
		long nowTolerance() {
			return NOW_TOLERANCE_SECONDS;
		}

		@Override
		int[] expectedMainnetBip32Path() {
			return MAINNET_BIP32_PATH;
		}

		@Override
		int[] expectedTestnetBip32Path() {
			return TESTNET_BIP32_PATH;
		}

		@Override
		Class<? extends KeyPair> keyPairClass() {
			return org.symbol.sdk.nem.KeyPair.class;
		}

		@Override
		KeyPair createFacadeKeyPair() {
			return FacadeFactoryTest.createFacadeKeyPair(org.symbol.sdk.nem.KeyPair::new);
		}

		@Override
		Account<?, ?> createFacadeAccount() {
			return facade.createAccount(TEST_PRIVATE_KEY);
		}

		@Override
		PublicAccount createFacadePublicAccount(final CryptoTypes.PublicKey publicKey) {
			return facade.createPublicAccount(publicKey);
		}

		@Override
		long facadeNow() {
			return facade.now().timestamp;
		}

		@Override
		CryptoTypes.Signature facadeSign(final CatbufferType transaction) {
			return facade.signTransaction(new org.symbol.sdk.nem.KeyPair(TEST_PRIVATE_KEY), asModel(transaction));
		}

		@Override
		boolean facadeVerify(final CatbufferType transaction, final CryptoTypes.Signature signature) {
			return facade.verifyTransaction(asModel(transaction), signature);
		}

		@Override
		CryptoTypes.Hash256 facadeHash(final CatbufferType transaction) {
			return facade.hashTransaction(asModel(transaction));
		}

		@Override
		byte[] facadeSigningPayload(final CatbufferType transaction) {
			return facade.extractSigningPayload(asModel(transaction));
		}

		@Override
		Verifier createChainVerifier(final CryptoTypes.PublicKey publicKey) {
			return new org.symbol.sdk.nem.Verifier(publicKey);
		}

		@Test
		void adapterAppliesAbsoluteFee() {
			// Arrange:
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();
			final CryptoTypes.PublicKey signerPublicKey = adapter.createKeyPair(TEST_PRIVATE_KEY).getPublicKey();

			// Act:
			final org.symbol.sdk.nem.models.Transaction transaction = asModel(
					adapter.createTransactionFromJson(TRANSFER_JSON, signerPublicKey, ABSOLUTE_FEE, TRANSACTION_DEADLINE_SECONDS));

			// Assert: NEM fee is absolute, proving the fee argument landed unchanged
			assertThat(transaction.getFee().value(), is(equalTo(ABSOLUTE_FEE)));
		}

		@Test
		void adapterAppliesDeadline() {
			// Arrange:
			final BlockchainFacade<?, ?, ?> adapter = createAdapter();
			final CryptoTypes.PublicKey signerPublicKey = adapter.createKeyPair(TEST_PRIVATE_KEY).getPublicKey();
			final NetworkTimestamp.Base now = adapter.now();

			// Act:
			final org.symbol.sdk.nem.models.Transaction transaction = asModel(
					adapter.createTransactionFromJson(TRANSFER_JSON, signerPublicKey, ABSOLUTE_FEE, TRANSACTION_DEADLINE_SECONDS));

			// Assert: deadline is deadlineSeconds past the timestamp
			assertThat(transaction.getDeadline().value() - transaction.getTimestamp().value(), is(equalTo(TRANSACTION_DEADLINE_SECONDS)));

			final long minDeadline = now.timestamp + TRANSACTION_DEADLINE_SECONDS;
			assertThat(transaction.getDeadline().value(), is(greaterThanOrEqualTo(minDeadline)));
			assertThat(transaction.getDeadline().value(), is(lessThanOrEqualTo(minDeadline + DEADLINE_SCHEDULING_DRIFT_SECONDS)));
		}
	}
}
