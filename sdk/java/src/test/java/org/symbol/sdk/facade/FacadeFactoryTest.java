package org.symbol.sdk.facade;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.greaterThanOrEqualTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.lessThanOrEqualTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.CatbufferType;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.NetworkTimestamp;
import org.symbol.sdk.TypedDescriptor;
import org.symbol.sdk.Verifier;

/**
 * Tests {@link FacadeFactory} and its {@link BlockchainFacade} adapters. (Java-only) — the runtime-selected facade abstraction has no
 * JS/Python counterpart; those SDKs choose a facade class dynamically.
 */
final class FacadeFactoryTest {
	private static final CryptoTypes.PrivateKey TEST_PRIVATE_KEY = new CryptoTypes.PrivateKey(
			"575DBB3062267EFF57C970A336EBBC8FBCFE12C5BD3ED7BC11EB0481D7704CED");

	private static final String SYMBOL_TRANSFER_JSON = FacadeTestData.SYMBOL_TRANSFER_JSON;

	private static final String NEM_TRANSFER_JSON = FacadeTestData.NEM_TRANSFER_JSON;

	// region create

	@Test
	void canCreateSymbolFacade() {
		// Act:
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("symbol", "testnet");

		// Assert:
		assertThat(facade.bip32CurveName(), is(equalTo(SymbolFacade.BIP32_CURVE_NAME)));
		assertThat(facade.network().name, is(equalTo("testnet")));
	}

	@Test
	void canCreateNemFacade() {
		// Act:
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("nem", "testnet");

		// Assert:
		assertThat(facade.bip32CurveName(), is(equalTo(NemFacade.BIP32_CURVE_NAME)));
		assertThat(facade.network().name, is(equalTo("testnet")));
	}

	@Test
	void createIsCaseInsensitive() {
		// Act:
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("Symbol", "mainnet");

		// Assert:
		assertThat(facade.bip32CurveName(), is(equalTo(SymbolFacade.BIP32_CURVE_NAME)));
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

	// region generic flow

	private static <T extends CatbufferType, D extends TypedDescriptor, K extends KeyPair> boolean signAndVerify(
			final BlockchainFacade<T, D, K> facade, final String json) {
		// Arrange:
		final Account<T, K> account = facade.createAccount(TEST_PRIVATE_KEY);
		final T transaction = facade.createTransactionFromJson(json, account.publicKey(), 100L, 3600L);

		// Act:
		final CryptoTypes.Signature signature = facade.signTransaction(account.keyPair(), transaction);
		final CryptoTypes.Signature accountSignature = account.signTransaction(transaction);

		// Assert: ed25519 signing is deterministic, so the account path must produce the identical signature
		assertThat(accountSignature, is(equalTo(signature)));
		return facade.verifyTransaction(transaction, signature);
	}

	@Test
	void canSignAndVerifySymbolTransactionThroughWildcardHandle() {
		// Act:
		final boolean isVerified = signAndVerify(FacadeFactory.create("symbol", "testnet"), SYMBOL_TRANSFER_JSON);

		// Assert:
		assertThat(isVerified, is(true));
	}

	@Test
	void canSignAndVerifyNemTransactionThroughWildcardHandle() {
		// Act:
		final boolean isVerified = signAndVerify(FacadeFactory.create("nem", "testnet"), NEM_TRANSFER_JSON);

		// Assert:
		assertThat(isVerified, is(true));
	}

	@Test
	void verifierFromFacadeVerifiesRawMessages() {
		// Arrange:
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("symbol", "testnet");
		final Account<?, ?> account = facade.createAccount(TEST_PRIVATE_KEY);
		final byte[] message = {
				1, 2, 3
		};
		final CryptoTypes.Signature signature = account.keyPair().sign(message);

		// Act:
		final Verifier verifier = facade.createVerifier(account.publicKey());
		final boolean isVerified = verifier.verify(message, signature);

		// Assert:
		assertThat(isVerified, is(true));
	}

	@Test
	void accountMatchesFacadeAccount() {
		// Arrange:
		final NemFacade real = new NemFacade("testnet");
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("nem", "testnet");

		// Act:
		final Account<?, ?> account = facade.createAccount(TEST_PRIVATE_KEY);
		final NemFacade.NemAccount realAccount = real.createAccount(TEST_PRIVATE_KEY);

		// Assert: the adapter account mirrors the real facade account, and its address round-trips through createAddress
		assertThat(account.publicKey(), is(equalTo(realAccount.publicKey())));
		assertThat(account.address(), is(equalTo(realAccount.address())));
		assertThat(account.keyPair().getPrivateKey(), is(equalTo(realAccount.keyPair().getPrivateKey())));
		assertThat(facade.createAddress(account.address().toString()), is(equalTo(account.address())));
	}

	@Test
	void bip32PathDependsOnNetwork() {
		// Act: same blockchain, different networks — only the coin-type element should change with the network
		final int[] mainnetPath = FacadeFactory.create("symbol", "mainnet").bip32Path(2);
		final int[] testnetPath = FacadeFactory.create("symbol", "testnet").bip32Path(2);

		// Assert: symbol coin-type is 4343 on mainnet, 1 on testnet
		assertThat(mainnetPath, is(equalTo(new int[]{
				44, 4343, 2, 0, 0
		})));
		assertThat(testnetPath, is(equalTo(new int[]{
				44, 1, 2, 0, 0
		})));
	}

	@Test
	void publicAccountMatchesAccount() {
		// Arrange:
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("symbol", "testnet");
		final Account<?, ?> account = facade.createAccount(TEST_PRIVATE_KEY);

		// Act:
		final PublicAccount publicAccount = facade.createPublicAccount(account.publicKey());

		// Assert:
		assertThat(publicAccount.publicKey(), is(equalTo(account.publicKey())));
		assertThat(publicAccount.address(), is(equalTo(account.address())));
	}

	@Test
	void nemNowMatchesFacadeNetworkTime() {
		// Arrange:
		final NemFacade real = new NemFacade("testnet");
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("nem", "testnet");

		// Act: both derive network time from the same epoch, called microseconds apart
		final long adapterNow = facade.now().timestamp;
		final long facadeNow = real.now().timestamp;

		// Assert: the adapter delegates now() to the facade (NEM timestamps are seconds — allow a small wall-clock window)
		assertThat(Math.abs(adapterNow - facadeNow), is(lessThanOrEqualTo(5L)));
	}

	@Test
	void symbolNowMatchesFacadeNetworkTime() {
		// Arrange:
		final SymbolFacade real = new SymbolFacade("testnet");
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("symbol", "testnet");

		// Act:
		final long adapterNow = facade.now().timestamp;
		final long facadeNow = real.now().timestamp;

		// Assert: the adapter delegates now() to the facade (Symbol timestamps are milliseconds)
		assertThat(Math.abs(adapterNow - facadeNow), is(lessThanOrEqualTo(5_000L)));
	}

	/** A transaction built through an adapter, plus that adapter's hash and signing payload for it. */
	private record BuiltTransaction(CatbufferType transaction, CryptoTypes.Hash256 hash, byte[] signingPayload) {
	}

	// build the transaction and take its hash / signing payload through the adapter (the wildcard capture ties them together)
	private static <T extends CatbufferType, D extends TypedDescriptor, K extends KeyPair> BuiltTransaction buildThroughAdapter(
			final BlockchainFacade<T, D, K> facade, final String json, final CryptoTypes.PublicKey signerPublicKey) {
		final T transaction = facade.createTransactionFromJson(json, signerPublicKey, 100L, 3600L);
		return new BuiltTransaction(transaction, facade.hashTransaction(transaction), facade.extractSigningPayload(transaction));
	}

	@Test
	void symbolAdapterHashAndSigningPayloadMatchFacade() {
		// Arrange:
		final SymbolFacade real = new SymbolFacade("testnet");
		final CryptoTypes.PublicKey signerPublicKey = new org.symbol.sdk.symbol.KeyPair(TEST_PRIVATE_KEY).getPublicKey();

		// Act:
		final BuiltTransaction built = buildThroughAdapter(FacadeFactory.create("symbol", "testnet"), SYMBOL_TRANSFER_JSON,
				signerPublicKey);
		final org.symbol.sdk.symbol.models.Transaction transaction = (org.symbol.sdk.symbol.models.Transaction) built.transaction();

		// Assert: the adapter delegates hashTransaction / extractSigningPayload to the real facade (identical for the same tx)
		assertThat(built.hash(), is(equalTo(real.hashTransaction(transaction))));
		assertThat(built.signingPayload(), is(equalTo(real.extractSigningPayload(transaction))));
	}

	@Test
	void nemAdapterHashAndSigningPayloadMatchFacade() {
		// Arrange:
		final NemFacade real = new NemFacade("testnet");
		final CryptoTypes.PublicKey signerPublicKey = new org.symbol.sdk.nem.KeyPair(TEST_PRIVATE_KEY).getPublicKey();

		// Act:
		final BuiltTransaction built = buildThroughAdapter(FacadeFactory.create("nem", "testnet"), NEM_TRANSFER_JSON, signerPublicKey);
		final org.symbol.sdk.nem.models.Transaction transaction = (org.symbol.sdk.nem.models.Transaction) built.transaction();

		// Assert:
		assertThat(built.hash(), is(equalTo(real.hashTransaction(transaction))));
		assertThat(built.signingPayload(), is(equalTo(real.extractSigningPayload(transaction))));
	}

	@Test
	void createKeyPairIsChainTyped() {
		// Act:
		final KeyPair symbolKeyPair = FacadeFactory.create("symbol", "testnet").createKeyPair(TEST_PRIVATE_KEY);
		final KeyPair nemKeyPair = FacadeFactory.create("nem", "testnet").createKeyPair(TEST_PRIVATE_KEY);

		// Assert: chain-typed, and deriving the same public key as the concrete SDK key pair
		assertThat(symbolKeyPair, is(instanceOf(org.symbol.sdk.symbol.KeyPair.class)));
		assertThat(symbolKeyPair.getPublicKey(), is(equalTo(new org.symbol.sdk.symbol.KeyPair(TEST_PRIVATE_KEY).getPublicKey())));
		assertThat(nemKeyPair, is(instanceOf(org.symbol.sdk.nem.KeyPair.class)));
		assertThat(nemKeyPair.getPublicKey(), is(equalTo(new org.symbol.sdk.nem.KeyPair(TEST_PRIVATE_KEY).getPublicKey())));
	}

	@Test
	void symbolAdapterAppliesFeeMultiplierAndDeadline() {
		// Arrange:
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("symbol", "testnet");
		final CryptoTypes.PublicKey signerPublicKey = facade.createKeyPair(TEST_PRIVATE_KEY).getPublicKey();
		final NetworkTimestamp.Base now = facade.now();

		// Act:
		final org.symbol.sdk.symbol.models.Transaction transaction = (org.symbol.sdk.symbol.models.Transaction) facade
				.createTransactionFromJson(SYMBOL_TRANSFER_JSON, signerPublicKey, 100L, 3600L);

		// Assert: Symbol fee is feeMultiplier * size, proving 100 landed as the multiplier (not the deadline)
		assertThat(transaction.getFee().value(), is(equalTo((long) transaction.size() * 100L)));

		final long minDeadline = now.timestamp + 3600L * 1000L;
		assertThat(transaction.getDeadline().value(), is(greaterThanOrEqualTo(minDeadline)));
		assertThat(transaction.getDeadline().value(), is(lessThanOrEqualTo(minDeadline + 10_000L)));
	}

	@Test
	void nemAdapterAppliesAbsoluteFeeAndDeadline() {
		// Arrange:
		final BlockchainFacade<?, ?, ?> facade = FacadeFactory.create("nem", "testnet");
		final CryptoTypes.PublicKey signerPublicKey = facade.createKeyPair(TEST_PRIVATE_KEY).getPublicKey();

		// Act:
		final org.symbol.sdk.nem.models.Transaction transaction = (org.symbol.sdk.nem.models.Transaction) facade
				.createTransactionFromJson(NEM_TRANSFER_JSON, signerPublicKey, 100000L, 3600L);

		// Assert: NEM fee is absolute (proving 100000 landed as the fee), and deadline is deadlineSeconds past the timestamp
		assertThat(transaction.getFee().value(), is(equalTo(100000L)));
		assertThat(transaction.getDeadline().value() - transaction.getTimestamp().value(), is(equalTo(3600L)));
	}

	// endregion
}
