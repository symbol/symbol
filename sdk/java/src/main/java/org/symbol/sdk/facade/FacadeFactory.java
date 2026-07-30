package org.symbol.sdk.facade;

import java.util.Locale;

import org.symbol.sdk.Bip32;
import org.symbol.sdk.ByteArray;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.Network;
import org.symbol.sdk.NetworkTimestamp;
import org.symbol.sdk.Verifier;
import org.symbol.sdk.nem.descriptors.NemTransactionDescriptor;
import org.symbol.sdk.symbol.descriptors.SymbolTransactionDescriptor;

/**
 * Creates a {@link BlockchainFacade} for a blockchain chosen at runtime. Each handle is a thin adapter that delegates to a freshly
 * constructed chain facade; the chain-specific type parameters make a foreign chain's transaction or key pair a compile error. Call the
 * transaction-typed methods from a generic capture-helper method (see the {@link BlockchainFacade} example); the account, network and BIP32
 * surface is usable on the wildcard handle directly.
 */
public final class FacadeFactory {
	private FacadeFactory() {
	}

	/**
	 * Creates a facade for the named blockchain and network.
	 *
	 * @param blockchain Blockchain name ({@code "symbol"} or {@code "nem"}, case-insensitive).
	 * @param networkName Network name (e.g. {@code "mainnet"}, {@code "testnet"}).
	 * @return Facade for the blockchain, typed as the blockchain-agnostic interface.
	 */
	public static BlockchainFacade<?, ?, ?> create(final String blockchain, final String networkName) {
		if (null == blockchain)
			throw new IllegalArgumentException("blockchain name is required");

		return switch (blockchain.toLowerCase(Locale.ROOT)) {
			case "symbol" -> new SymbolAdapter(new SymbolFacade(networkName));
			case "nem" -> new NemAdapter(new NemFacade(networkName));
			default -> throw new IllegalArgumentException("unknown blockchain: " + blockchain);
		};
	}

	private record SymbolAdapter(SymbolFacade facade)
			implements
				BlockchainFacade<org.symbol.sdk.symbol.models.Transaction, SymbolTransactionDescriptor, org.symbol.sdk.symbol.KeyPair> {
		@Override
		public Network<?, ?> network() {
			return facade.network;
		}

		@Override
		public NetworkTimestamp.Base now() {
			return facade.now();
		}

		@Override
		public String bip32CurveName() {
			return SymbolFacade.BIP32_CURVE_NAME;
		}

		@Override
		public int[] bip32Path(final int accountId) {
			return facade.bip32Path(accountId);
		}

		@Override
		public org.symbol.sdk.symbol.KeyPair bip32NodeToKeyPair(final Bip32.Bip32Node bip32Node) {
			return SymbolFacade.bip32NodeToKeyPair(bip32Node);
		}

		@Override
		public org.symbol.sdk.symbol.KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
			return new org.symbol.sdk.symbol.KeyPair(privateKey);
		}

		@Override
		public CryptoTypes.SharedKey256 deriveSharedKey(final org.symbol.sdk.symbol.KeyPair keyPair,
				final CryptoTypes.PublicKey otherPublicKey) {
			return org.symbol.sdk.symbol.SharedKey.deriveSharedKey(keyPair, otherPublicKey);
		}

		@Override
		public Verifier createVerifier(final CryptoTypes.PublicKey publicKey) {
			return new org.symbol.sdk.symbol.Verifier(publicKey);
		}

		@Override
		public ByteArray createAddress(final String address) {
			return new org.symbol.sdk.symbol.Address(address);
		}

		@Override
		public SymbolFacade.SymbolPublicAccount createPublicAccount(final CryptoTypes.PublicKey publicKey) {
			return facade.createPublicAccount(publicKey);
		}

		@Override
		public SymbolFacade.SymbolAccount createAccount(final CryptoTypes.PrivateKey privateKey) {
			return facade.createAccount(privateKey);
		}

		@Override
		public org.symbol.sdk.symbol.models.Transaction createTransactionFromTypedDescriptor(
				final SymbolTransactionDescriptor typedDescriptor, final CryptoTypes.PublicKey signerPublicKey, final long fee,
				final long deadlineSeconds) {
			return facade.createTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey, fee, deadlineSeconds);
		}

		@Override
		public org.symbol.sdk.symbol.models.Transaction createTransactionFromJson(final String json,
				final CryptoTypes.PublicKey signerPublicKey, final long fee, final long deadlineSeconds) {
			return facade.createTransactionFromJson(json, signerPublicKey, fee, deadlineSeconds);
		}

		@Override
		public CryptoTypes.Hash256 hashTransaction(final org.symbol.sdk.symbol.models.Transaction transaction) {
			return facade.hashTransaction(transaction);
		}

		@Override
		public byte[] extractSigningPayload(final org.symbol.sdk.symbol.models.Transaction transaction) {
			return facade.extractSigningPayload(transaction);
		}

		@Override
		public CryptoTypes.Signature signTransaction(final org.symbol.sdk.symbol.KeyPair keyPair,
				final org.symbol.sdk.symbol.models.Transaction transaction) {
			return facade.signTransaction(keyPair, transaction);
		}

		@Override
		public boolean verifyTransaction(final org.symbol.sdk.symbol.models.Transaction transaction,
				final CryptoTypes.Signature signature) {
			return facade.verifyTransaction(transaction, signature);
		}
	}

	private record NemAdapter(NemFacade facade)
			implements
				BlockchainFacade<org.symbol.sdk.nem.models.Transaction, NemTransactionDescriptor, org.symbol.sdk.nem.KeyPair> {
		@Override
		public Network<?, ?> network() {
			return facade.network;
		}

		@Override
		public NetworkTimestamp.Base now() {
			return facade.now();
		}

		@Override
		public String bip32CurveName() {
			return NemFacade.BIP32_CURVE_NAME;
		}

		@Override
		public int[] bip32Path(final int accountId) {
			return facade.bip32Path(accountId);
		}

		@Override
		public org.symbol.sdk.nem.KeyPair bip32NodeToKeyPair(final Bip32.Bip32Node bip32Node) {
			return NemFacade.bip32NodeToKeyPair(bip32Node);
		}

		@Override
		public org.symbol.sdk.nem.KeyPair createKeyPair(final CryptoTypes.PrivateKey privateKey) {
			return new org.symbol.sdk.nem.KeyPair(privateKey);
		}

		@Override
		public CryptoTypes.SharedKey256 deriveSharedKey(final org.symbol.sdk.nem.KeyPair keyPair,
				final CryptoTypes.PublicKey otherPublicKey) {
			return org.symbol.sdk.nem.SharedKey.deriveSharedKey(keyPair, otherPublicKey);
		}

		@Override
		public Verifier createVerifier(final CryptoTypes.PublicKey publicKey) {
			return new org.symbol.sdk.nem.Verifier(publicKey);
		}

		@Override
		public ByteArray createAddress(final String address) {
			return new org.symbol.sdk.nem.Address(address);
		}

		@Override
		public NemFacade.NemPublicAccount createPublicAccount(final CryptoTypes.PublicKey publicKey) {
			return facade.createPublicAccount(publicKey);
		}

		@Override
		public NemFacade.NemAccount createAccount(final CryptoTypes.PrivateKey privateKey) {
			return facade.createAccount(privateKey);
		}

		@Override
		public org.symbol.sdk.nem.models.Transaction createTransactionFromTypedDescriptor(final NemTransactionDescriptor typedDescriptor,
				final CryptoTypes.PublicKey signerPublicKey, final long fee, final long deadlineSeconds) {
			return facade.createTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey, fee, deadlineSeconds);
		}

		@Override
		public org.symbol.sdk.nem.models.Transaction createTransactionFromJson(final String json,
				final CryptoTypes.PublicKey signerPublicKey, final long fee, final long deadlineSeconds) {
			return facade.createTransactionFromJson(json, signerPublicKey, fee, deadlineSeconds);
		}

		@Override
		public CryptoTypes.Hash256 hashTransaction(final org.symbol.sdk.nem.models.Transaction transaction) {
			return facade.hashTransaction(transaction);
		}

		@Override
		public byte[] extractSigningPayload(final org.symbol.sdk.nem.models.Transaction transaction) {
			return facade.extractSigningPayload(transaction);
		}

		@Override
		public CryptoTypes.Signature signTransaction(final org.symbol.sdk.nem.KeyPair keyPair,
				final org.symbol.sdk.nem.models.Transaction transaction) {
			return facade.signTransaction(keyPair, transaction);
		}

		@Override
		public boolean verifyTransaction(final org.symbol.sdk.nem.models.Transaction transaction, final CryptoTypes.Signature signature) {
			return facade.verifyTransaction(transaction, signature);
		}
	}
}
