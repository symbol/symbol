package org.symbol.sdk.facade;

import org.symbol.sdk.Bip32;
import org.symbol.sdk.ByteArray;
import org.symbol.sdk.CatbufferType;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.KeyPair;
import org.symbol.sdk.Network;
import org.symbol.sdk.NetworkTimestamp;
import org.symbol.sdk.TypedDescriptor;
import org.symbol.sdk.Verifier;

/**
 * Blockchain-agnostic surface shared by {@link SymbolFacade} and {@link NemFacade}: build, sign, hash and verify transactions through a
 * single handle. The transaction, typed-descriptor and key-pair types stay chain-specific, so passing another chain's transaction or key
 * pair is a compile error; a key pair enters the wildcard world through the facade ({@link #createKeyPair}, {@link #bip32NodeToKeyPair},
 * {@link Account#keyPair()}). Each facade also exposes chain-specific methods (e.g. Symbol's embedded-transaction and cosignature helpers)
 * outside this contract.
 *
 * <p>
 * A facade obtained from {@link FacadeFactory#create} is typed {@code BlockchainFacade<?, ?, ?>}; the transaction-typed methods are then
 * called from a generic capture-helper method, where the wildcard is fixed to a consistent type:
 *
 * <pre>{@code
 * BlockchainFacade<?, ?, ?> facade = FacadeFactory.create(chainName, "testnet");
 * transfer(facade, json, privateKey);
 *
 * static <T extends CatbufferType, D extends TypedDescriptor, K extends KeyPair> void transfer(
 * 		BlockchainFacade<T, D, K> facade, String json, CryptoTypes.PrivateKey privateKey) {
 * 	Account<T, K> account = facade.createAccount(privateKey);
 * 	T transaction = facade.createTransactionFromJson(json, account.publicKey(), 100, 3600);
 * 	CryptoTypes.Signature signature = facade.signTransaction(account.keyPair(), transaction);
 * 	facade.verifyTransaction(transaction, signature);
 * }
 * }</pre>
 *
 * @param <TTransaction> Transaction model type.
 * @param <TDescriptor> Typed transaction-descriptor type.
 * @param <TKeyPair> Key-pair type.
 */
public interface BlockchainFacade<TTransaction extends CatbufferType, TDescriptor extends TypedDescriptor, TKeyPair extends KeyPair> {
	/**
	 * Gets the network this facade is bound to.
	 *
	 * @return Network.
	 */
	Network<?, ?> network();

	/**
	 * Gets the current network timestamp.
	 *
	 * @return Network timestamp.
	 */
	NetworkTimestamp.Base now();

	/**
	 * Gets the BIP32 curve name used by this blockchain.
	 *
	 * @return BIP32 curve name.
	 */
	String bip32CurveName();

	/**
	 * Builds a BIP32 derivation path for an account.
	 *
	 * @param accountId Account id.
	 * @return BIP32 path components.
	 */
	int[] bip32Path(int accountId);

	/**
	 * Derives a key pair from a BIP32 node using this blockchain's derivation rules.
	 *
	 * @param bip32Node BIP32 node.
	 * @return Key pair.
	 */
	TKeyPair bip32NodeToKeyPair(Bip32.Bip32Node bip32Node);

	/**
	 * Creates a key pair from a private key.
	 *
	 * @param privateKey Private key.
	 * @return Key pair.
	 */
	TKeyPair createKeyPair(CryptoTypes.PrivateKey privateKey);

	/**
	 * Derives a shared encryption key between one of this blockchain's key pairs and another party's public key.
	 *
	 * @param keyPair Key pair.
	 * @param otherPublicKey Other party's public key.
	 * @return Shared encryption key.
	 */
	CryptoTypes.SharedKey256 deriveSharedKey(TKeyPair keyPair, CryptoTypes.PublicKey otherPublicKey);

	/**
	 * Creates a verifier around a public key.
	 *
	 * @param publicKey Signer public key.
	 * @return Verifier.
	 */
	Verifier createVerifier(CryptoTypes.PublicKey publicKey);

	/**
	 * Parses an address from its canonical string form.
	 *
	 * @param address Address string.
	 * @return Address.
	 */
	ByteArray createAddress(String address);

	/**
	 * Creates a public account from a public key.
	 *
	 * @param publicKey Account public key.
	 * @return Public account.
	 */
	PublicAccount createPublicAccount(CryptoTypes.PublicKey publicKey);

	/**
	 * Creates an account from a private key.
	 *
	 * @param privateKey Account private key.
	 * @return Account.
	 */
	Account<TTransaction, TKeyPair> createAccount(CryptoTypes.PrivateKey privateKey);

	/**
	 * Creates a transaction from a typed descriptor.
	 *
	 * @param typedDescriptor Typed transaction descriptor.
	 * @param signerPublicKey Signer public key.
	 * @param fee Fee — a per-byte fee multiplier for Symbol, an absolute fee for NEM.
	 * @param deadlineSeconds Relative deadline in seconds.
	 * @return Transaction.
	 */
	TTransaction createTransactionFromTypedDescriptor(TDescriptor typedDescriptor, CryptoTypes.PublicKey signerPublicKey, long fee,
			long deadlineSeconds);

	/**
	 * Creates a transaction from a JSON descriptor.
	 *
	 * @param json JSON transaction descriptor.
	 * @param signerPublicKey Signer public key.
	 * @param fee Fee — a per-byte fee multiplier for Symbol, an absolute fee for NEM.
	 * @param deadlineSeconds Relative deadline in seconds.
	 * @return Transaction.
	 */
	TTransaction createTransactionFromJson(String json, CryptoTypes.PublicKey signerPublicKey, long fee, long deadlineSeconds);

	/**
	 * Hashes a transaction.
	 *
	 * @param transaction Transaction.
	 * @return Transaction hash.
	 */
	CryptoTypes.Hash256 hashTransaction(TTransaction transaction);

	/**
	 * Extracts the signing payload of a transaction.
	 *
	 * @param transaction Transaction.
	 * @return Signing payload bytes.
	 */
	byte[] extractSigningPayload(TTransaction transaction);

	/**
	 * Signs a transaction with this blockchain's key-pair type — the type parameter makes a foreign chain's key pair a compile error.
	 *
	 * @param keyPair Signer key pair.
	 * @param transaction Transaction.
	 * @return Signature.
	 */
	CryptoTypes.Signature signTransaction(TKeyPair keyPair, TTransaction transaction);

	/**
	 * Verifies a transaction signature.
	 *
	 * @param transaction Transaction.
	 * @param signature Signature.
	 * @return true if the signature is valid, false otherwise.
	 */
	boolean verifyTransaction(TTransaction transaction, CryptoTypes.Signature signature);
}
