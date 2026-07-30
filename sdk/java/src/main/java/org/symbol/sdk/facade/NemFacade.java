package org.symbol.sdk.facade;

import java.time.Instant;
import java.util.HashMap;
import java.util.Map;

import org.symbol.sdk.Bip32;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.JsonDescriptor;
import org.symbol.sdk.Network.NetworkLocator;
import org.symbol.sdk.nem.Address;
import org.symbol.sdk.nem.KeyPair;
import org.symbol.sdk.nem.MessageEncoder;
import org.symbol.sdk.nem.NemTransactionFactory;
import org.symbol.sdk.nem.Network;
import org.symbol.sdk.nem.NetworkTimestamp;
import org.symbol.sdk.nem.Verifier;
import org.symbol.sdk.nem.descriptors.NemTransactionDescriptor;
import org.symbol.sdk.nem.models.*;
import org.symbol.sdk.utils.ArrayHelpers;
import org.symbol.sdk.utils.Transforms;

/**
 * High-level facade used to interact with the NEM blockchain: signing, verifying, hashing, and creating transactions.
 */
public final class NemFacade {

	/** BIP32 curve name for NEM. */
	public static final String BIP32_CURVE_NAME = "ed25519-keccak";

	/** Underlying network. */
	public final Network network;

	/** Underlying transaction factory. */
	public final NemTransactionFactory transactionFactory;

	/**
	 * Creates a NEM facade around an explicit network.
	 *
	 * @param network NEM network.
	 */
	public NemFacade(final Network network) {
		this.network = network;
		this.transactionFactory = new NemTransactionFactory(network);
	}

	/**
	 * Creates a NEM facade by network name.
	 *
	 * @param networkName Network name (e.g. {@code "mainnet"}, {@code "testnet"}).
	 */
	public NemFacade(final String networkName) {
		this(NetworkLocator.findByName(Network.NETWORKS, networkName));
	}

	/**
	 * Creates a network timestamp representing the current time.
	 *
	 * @return Network timestamp representing the current time.
	 */
	public NetworkTimestamp now() {
		return network.fromDatetime(Instant.now());
	}

	/**
	 * Creates a NEM public account from a public key.
	 *
	 * @param publicKey Account public key.
	 * @return NEM public account.
	 */
	public NemPublicAccount createPublicAccount(final CryptoTypes.PublicKey publicKey) {
		return new NemPublicAccount(this, publicKey);
	}

	/**
	 * Creates a NEM account from a private key.
	 *
	 * @param privateKey Account private key.
	 * @return NEM account.
	 */
	public NemAccount createAccount(final CryptoTypes.PrivateKey privateKey) {
		return new NemAccount(this, new KeyPair(privateKey));
	}

	/**
	 * Creates a transaction from a descriptor, adding signer, fee, timestamp and deadline to a copy of it.
	 *
	 * @param descriptor Transaction descriptor (must contain {@code "type"}); not mutated.
	 * @param signerPublicKey Signer public key.
	 * @param fee Transaction fee.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @return Created transaction.
	 */
	Transaction createTransactionFromDescriptor(final Map<String, Object> descriptor, final CryptoTypes.PublicKey signerPublicKey,
			final long fee, final long deadlineSeconds) {
		final NetworkTimestamp current = now();
		final Map<String, Object> rawDescriptor = new HashMap<>(descriptor);
		rawDescriptor.put("signerPublicKey", signerPublicKey);
		rawDescriptor.put("fee", fee);
		rawDescriptor.put("timestamp", current.timestamp);
		rawDescriptor.put("deadline", current.addSeconds(deadlineSeconds).timestamp);
		return transactionFactory.create(rawDescriptor);
	}

	/**
	 * Creates a transaction from a typed transaction descriptor.
	 *
	 * @param typedDescriptor Typed transaction descriptor.
	 * @param signerPublicKey Signer public key.
	 * @param fee Transaction fee.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @return Created transaction.
	 */
	public Transaction createTransactionFromTypedDescriptor(final NemTransactionDescriptor typedDescriptor,
			final CryptoTypes.PublicKey signerPublicKey, final long fee, final long deadlineSeconds) {
		return createTransactionFromDescriptor(typedDescriptor.toMap(), signerPublicKey, fee, deadlineSeconds);
	}

	/**
	 * Creates a transaction from a JSON descriptor document.
	 *
	 * @param json JSON object with the same shape as the untyped descriptor map.
	 * @param signerPublicKey Signer public key.
	 * @param fee Transaction fee.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @return Created transaction.
	 */
	public Transaction createTransactionFromJson(final String json, final CryptoTypes.PublicKey signerPublicKey, final long fee,
			final long deadlineSeconds) {
		return createTransactionFromDescriptor(JsonDescriptor.parse(json), signerPublicKey, fee, deadlineSeconds);
	}

	// the following three functions are NOT static in order for NemFacade and SymbolFacade to conform to the same interface

	/**
	 * Hashes a NEM transaction; the hash is Keccak-256 of the non-verifiable serialization.
	 *
	 * @param transaction Transaction object.
	 * @return Transaction hash.
	 */
	public CryptoTypes.Hash256 hashTransaction(final Transaction transaction) {
		final byte[] payload = NemTransactionFactory.toNonVerifiableTransaction(transaction).serialize();
		return new CryptoTypes.Hash256(Transforms.keccak_256(payload));
	}

	/**
	 * Gets the payload to sign for a NEM transaction.
	 *
	 * @param transaction Transaction object.
	 * @return Verifiable data to sign — the non-verifiable serialization of the transaction.
	 */
	public byte[] extractSigningPayload(final Transaction transaction) {
		return NemTransactionFactory.toNonVerifiableTransaction(transaction).serialize();
	}

	/**
	 * Signs a NEM transaction.
	 *
	 * @param keyPair Signer key pair.
	 * @param transaction Transaction object.
	 * @return Transaction signature.
	 */
	public CryptoTypes.Signature signTransaction(final KeyPair keyPair, final Transaction transaction) {
		return keyPair.sign(extractSigningPayload(transaction));
	}

	/**
	 * Verifies a NEM transaction signature.
	 *
	 * @param transaction Transaction object.
	 * @param signature Signature to verify.
	 * @return {@code true} if the signature is valid for the transaction.
	 */
	public boolean verifyTransaction(final Transaction transaction, final CryptoTypes.Signature signature) {
		return new Verifier(new CryptoTypes.PublicKey(transaction.getSignerPublicKey().bytes())).verify(extractSigningPayload(transaction),
				signature);
	}

	/**
	 * Creates a network-compatible BIP32 path for the specified account.
	 *
	 * @param accountId Id of the account for which to generate a BIP32 path.
	 * @return BIP32 path for the specified account.
	 */
	public int[] bip32Path(final int accountId) {
		final int coinType = "mainnet".equals(network.name) ? 43 : 1;
		return new int[]{
				44, coinType, accountId, 0, 0
		};
	}

	/**
	 * Derives a NEM {@link KeyPair} from a BIP32 node. BIP32 private keys should be used as-is, so the bytes are reversed here to
	 * counteract the NEM {@link KeyPair} constructor's internal reverse.
	 *
	 * @param bip32Node BIP32 node.
	 * @return Derived key pair.
	 */
	public static KeyPair bip32NodeToKeyPair(final Bip32.Bip32Node bip32Node) {
		return new KeyPair(new CryptoTypes.PrivateKey(ArrayHelpers.reverse(bip32Node.privateKey.bytes())));
	}

	// region account wrappers

	/**
	 * NEM public account — known public key plus derived address.
	 */
	public static class NemPublicAccount implements PublicAccount {
		/** Owning facade. */
		protected final NemFacade facade;

		/** Account public key. */
		private final CryptoTypes.PublicKey publicKey;

		/** Account address. */
		private final Address address;

		NemPublicAccount(final NemFacade facade, final CryptoTypes.PublicKey publicKey) {
			this.facade = facade;
			this.publicKey = publicKey;
			this.address = facade.network.publicKeyToAddress(publicKey);
		}

		@Override
		public CryptoTypes.PublicKey publicKey() {
			return publicKey;
		}

		@Override
		public Address address() {
			return address;
		}
	}

	/**
	 * NEM account — adds the signing key pair to {@link NemPublicAccount}.
	 */
	public static final class NemAccount extends NemPublicAccount implements Account<Transaction, KeyPair> {
		/** Account key pair. */
		private final KeyPair keyPair;

		NemAccount(final NemFacade facade, final KeyPair keyPair) {
			super(facade, keyPair.getPublicKey());
			this.keyPair = keyPair;
		}

		@Override
		public KeyPair keyPair() {
			return keyPair;
		}

		/**
		 * Creates a {@link MessageEncoder} using this account as one party.
		 *
		 * @return Message encoder.
		 */
		public MessageEncoder messageEncoder() {
			return new MessageEncoder(this.keyPair);
		}

		/**
		 * Signs a NEM transaction.
		 *
		 * @param transaction Transaction object.
		 * @return Transaction signature.
		 */
		public CryptoTypes.Signature signTransaction(final Transaction transaction) {
			return this.facade.signTransaction(this.keyPair, transaction);
		}
	}

	// endregion
}
