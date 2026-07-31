package org.symbol.sdk.facade;

import java.time.Instant;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.symbol.sdk.Bip32;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.JsonDescriptor;
import org.symbol.sdk.Network.NetworkLocator;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.Merkle;
import org.symbol.sdk.symbol.MessageEncoder;
import org.symbol.sdk.symbol.Network;
import org.symbol.sdk.symbol.NetworkTimestamp;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.Verifier;
import org.symbol.sdk.symbol.descriptors.SymbolTransactionDescriptor;
import org.symbol.sdk.symbol.models.*;
import org.symbol.sdk.utils.ArrayHelpers;
import org.symbol.sdk.utils.Transforms;

/**
 * High-level facade for interacting with the Symbol blockchain: signing, verifying, hashing, cosigning, and creating transactions from
 * untyped {@code Map} descriptors or generated {@link SymbolTransactionDescriptor}s.
 */
public final class SymbolFacade {

	/** BIP32 curve name for Symbol. */
	public static final String BIP32_CURVE_NAME = "ed25519";

	// region transaction-data-buffer constants

	// verifiable-entity header: size(4) + reserved1(4) + signature + signer + reserved2(4)
	private static final int TRANSACTION_HEADER_SIZE = 4 + 4 + Signature.SIZE + PublicKey.SIZE + 4;

	// version+network+type(4) + maxFee(8) + deadline(8) + transactionsHash(32)
	private static final int PRE_V3_AGGREGATE_HASHED_SIZE = 4 + 8 + 8 + Hash256.SIZE;

	// adds payloadSize(4) on top
	private static final int AGGREGATE_HASHED_SIZE = PRE_V3_AGGREGATE_HASHED_SIZE + 4;

	// endregion

	/** Underlying network. */
	public final Network network;

	/** Underlying transaction factory. */
	public final SymbolTransactionFactory transactionFactory;

	/**
	 * Creates a Symbol facade around an explicit network.
	 *
	 * @param network Symbol network.
	 */
	public SymbolFacade(final Network network) {
		this.network = network;
		this.transactionFactory = new SymbolTransactionFactory(network);
	}

	/**
	 * Creates a Symbol facade by network name.
	 *
	 * @param networkName Network name (e.g. {@code "mainnet"}, {@code "testnet"}).
	 */
	public SymbolFacade(final String networkName) {
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
	 * Creates a Symbol public account from a public key.
	 *
	 * @param publicKey Account public key.
	 * @return Symbol public account.
	 */
	public SymbolPublicAccount createPublicAccount(final CryptoTypes.PublicKey publicKey) {
		return new SymbolPublicAccount(this, publicKey);
	}

	/**
	 * Creates a Symbol account from a private key.
	 *
	 * @param privateKey Account private key.
	 * @return Symbol account.
	 */
	public SymbolAccount createAccount(final CryptoTypes.PrivateKey privateKey) {
		return new SymbolAccount(this, new KeyPair(privateKey));
	}

	/**
	 * Creates a transaction from a descriptor. {@code signerPublicKey} and {@code deadline} are added/overwritten, and {@code fee} is
	 * computed as {@code (size + reservedCosignatures * cosignatureSize) * feeMultiplier}.
	 *
	 * @param descriptor Transaction descriptor (must contain {@code "type"}); not mutated.
	 * @param signerPublicKey Signer public key.
	 * @param feeMultiplier Fee multiplier.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @return Created transaction with fee filled in.
	 */
	Transaction createTransactionFromDescriptor(final Map<String, Object> descriptor, final CryptoTypes.PublicKey signerPublicKey,
			final long feeMultiplier, final long deadlineSeconds) {
		return createTransactionFromDescriptor(descriptor, signerPublicKey, feeMultiplier, deadlineSeconds, 0);
	}

	/**
	 * Creates a transaction from a descriptor, reserving space for the given number of cosignatures when computing the fee.
	 *
	 * @param descriptor Transaction descriptor.
	 * @param signerPublicKey Signer public key.
	 * @param feeMultiplier Fee multiplier.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @param cosignatureCount Number of cosignature spaces to reserve when computing the fee.
	 * @return Created transaction with fee filled in.
	 */
	Transaction createTransactionFromDescriptor(final Map<String, Object> descriptor, final CryptoTypes.PublicKey signerPublicKey,
			final long feeMultiplier, final long deadlineSeconds, final int cosignatureCount) {
		final Map<String, Object> rawDescriptor = new HashMap<>(descriptor);
		rawDescriptor.put("signerPublicKey", signerPublicKey);
		rawDescriptor.put("deadline", now().addSeconds(deadlineSeconds).timestamp);

		final Transaction transaction = transactionFactory.create(rawDescriptor);

		// if cosignatures are specified in the descriptor, use the max of them and cosignatureCount
		int cosignatureCountAdjustment = cosignatureCount;
		final Object explicitCosignatures = rawDescriptor.get("cosignatures");
		if (explicitCosignatures instanceof List<?> list)
			cosignatureCountAdjustment = list.size() > cosignatureCount ? 0 : cosignatureCount - list.size();

		final long transactionWithCosignaturesSize = (long) transaction.size()
				+ (long) cosignatureCountAdjustment * new Cosignature().size();
		transaction.setFee(new Amount(transactionWithCosignaturesSize * feeMultiplier));
		return transaction;
	}

	/**
	 * Creates an embedded transaction from a descriptor.
	 *
	 * @param descriptor Transaction descriptor.
	 * @param signerPublicKey Signer public key.
	 * @return Created embedded transaction.
	 */
	EmbeddedTransaction createEmbeddedTransactionFromDescriptor(final Map<String, Object> descriptor,
			final CryptoTypes.PublicKey signerPublicKey) {
		final Map<String, Object> rawDescriptor = new HashMap<>(descriptor);
		rawDescriptor.put("signerPublicKey", signerPublicKey);
		return transactionFactory.createEmbedded(rawDescriptor);
	}

	/**
	 * Creates a transaction from a typed transaction descriptor, reserving no cosignature space.
	 *
	 * @param typedDescriptor Typed transaction descriptor.
	 * @param signerPublicKey Signer public key.
	 * @param feeMultiplier Fee multiplier.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @return Created transaction with fee filled in.
	 */
	public Transaction createTransactionFromTypedDescriptor(final SymbolTransactionDescriptor typedDescriptor,
			final CryptoTypes.PublicKey signerPublicKey, final long feeMultiplier, final long deadlineSeconds) {
		return createTransactionFromTypedDescriptor(typedDescriptor, signerPublicKey, feeMultiplier, deadlineSeconds, 0);
	}

	/**
	 * Creates a transaction from a typed transaction descriptor; fee / deadline / cosignature handling is shared with
	 * {@link #createTransactionFromDescriptor(Map, CryptoTypes.PublicKey, long, long, int)}.
	 *
	 * @param typedDescriptor Typed transaction descriptor.
	 * @param signerPublicKey Signer public key.
	 * @param feeMultiplier Fee multiplier.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @param cosignatureCount Number of cosignature spaces to reserve when computing the fee.
	 * @return Created transaction with fee filled in.
	 */
	public Transaction createTransactionFromTypedDescriptor(final SymbolTransactionDescriptor typedDescriptor,
			final CryptoTypes.PublicKey signerPublicKey, final long feeMultiplier, final long deadlineSeconds, final int cosignatureCount) {
		return createTransactionFromDescriptor(typedDescriptor.toMap(), signerPublicKey, feeMultiplier, deadlineSeconds, cosignatureCount);
	}

	/**
	 * Creates an embedded transaction from a typed transaction descriptor.
	 *
	 * @param typedDescriptor Typed transaction descriptor.
	 * @param signerPublicKey Signer public key.
	 * @return Created embedded transaction.
	 */
	public EmbeddedTransaction createEmbeddedTransactionFromTypedDescriptor(final SymbolTransactionDescriptor typedDescriptor,
			final CryptoTypes.PublicKey signerPublicKey) {
		return createEmbeddedTransactionFromDescriptor(typedDescriptor.toMap(), signerPublicKey);
	}

	/**
	 * Creates a transaction from a JSON descriptor document, reserving no cosignature space.
	 *
	 * @param json JSON object with the same shape as the untyped descriptor map.
	 * @param signerPublicKey Signer public key.
	 * @param feeMultiplier Fee multiplier.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @return Created transaction with fee filled in.
	 */
	public Transaction createTransactionFromJson(final String json, final CryptoTypes.PublicKey signerPublicKey, final long feeMultiplier,
			final long deadlineSeconds) {
		return createTransactionFromJson(json, signerPublicKey, feeMultiplier, deadlineSeconds, 0);
	}

	/**
	 * Creates a transaction from a JSON descriptor document.
	 *
	 * @param json JSON object with the same shape as the untyped descriptor map.
	 * @param signerPublicKey Signer public key.
	 * @param feeMultiplier Fee multiplier.
	 * @param deadlineSeconds Approximate seconds from now for the deadline.
	 * @param cosignatureCount Number of cosignature spaces to reserve when computing the fee.
	 * @return Created transaction with fee filled in.
	 */
	public Transaction createTransactionFromJson(final String json, final CryptoTypes.PublicKey signerPublicKey, final long feeMultiplier,
			final long deadlineSeconds, final int cosignatureCount) {
		return createTransactionFromDescriptor(JsonDescriptor.parse(json), signerPublicKey, feeMultiplier, deadlineSeconds,
				cosignatureCount);
	}

	/**
	 * Creates an embedded transaction from a JSON descriptor document.
	 *
	 * @param json JSON object with the same shape as the untyped descriptor map.
	 * @param signerPublicKey Signer public key.
	 * @return Created embedded transaction.
	 */
	public EmbeddedTransaction createEmbeddedTransactionFromJson(final String json, final CryptoTypes.PublicKey signerPublicKey) {
		return createEmbeddedTransactionFromDescriptor(JsonDescriptor.parse(json), signerPublicKey);
	}

	/**
	 * Hashes a Symbol transaction.
	 *
	 * @param transaction Transaction object.
	 * @return Transaction hash.
	 */
	public CryptoTypes.Hash256 hashTransaction(final Transaction transaction) {
		final byte[] out = Transforms.sha3_256(transaction.getSignature().bytes(), transaction.getSignerPublicKey().bytes(),
				network.generationHashSeed.bytes(), transactionDataBuffer(transaction.getType(), transaction.serialize()));
		return new CryptoTypes.Hash256(out);
	}

	/**
	 * Gets the verifiable payload to sign for a Symbol transaction.
	 *
	 * @param transaction Transaction object.
	 * @return Bytes to sign — {@code generationHashSeed | transactionDataBuffer}.
	 */
	public byte[] extractSigningPayload(final Transaction transaction) {
		final byte[] seed = network.generationHashSeed.bytes();
		final byte[] data = transactionDataBuffer(transaction.getType(), transaction.serialize());
		return ArrayHelpers.concat(seed, data);
	}

	/**
	 * Signs a Symbol transaction.
	 *
	 * @param keyPair Signer key pair.
	 * @param transaction Transaction object.
	 * @return Transaction signature.
	 */
	public CryptoTypes.Signature signTransaction(final KeyPair keyPair, final Transaction transaction) {
		return keyPair.sign(extractSigningPayload(transaction));
	}

	/**
	 * Verifies a Symbol transaction signature.
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
	 * Cosigns a Symbol transaction hash, producing a cosignature suitable for attaching to an aggregate.
	 *
	 * @param keyPair Key pair of the cosignatory.
	 * @param transactionHash Transaction hash.
	 * @return Signed cosignature.
	 */
	public static Cosignature cosignTransactionHash(final KeyPair keyPair, final CryptoTypes.Hash256 transactionHash) {
		final Cosignature cosignature = new Cosignature();
		cosignature.setVersion(0L);
		cosignature.setSignerPublicKey(new PublicKey(keyPair.getPublicKey().bytes()));
		cosignature.setSignature(new Signature(keyPair.sign(transactionHash.bytes()).bytes()));
		return cosignature;
	}

	/**
	 * Cosigns a Symbol transaction hash, producing a detached cosignature suitable for network propagation.
	 *
	 * @param keyPair Key pair of the cosignatory.
	 * @param transactionHash Transaction hash.
	 * @return Signed detached cosignature.
	 */
	public static DetachedCosignature cosignTransactionHashDetached(final KeyPair keyPair, final CryptoTypes.Hash256 transactionHash) {
		final DetachedCosignature cosignature = new DetachedCosignature();
		cosignature.setParentHash(new Hash256(transactionHash.bytes()));
		cosignature.setVersion(0L);
		cosignature.setSignerPublicKey(new PublicKey(keyPair.getPublicKey().bytes()));
		cosignature.setSignature(new Signature(keyPair.sign(transactionHash.bytes()).bytes()));
		return cosignature;
	}

	/**
	 * Cosigns a Symbol transaction, producing a cosignature suitable for attaching to an aggregate.
	 *
	 * @param keyPair Key pair of the cosignatory.
	 * @param transaction Transaction object.
	 * @return Signed cosignature.
	 */
	public Cosignature cosignTransaction(final KeyPair keyPair, final Transaction transaction) {
		return cosignTransactionHash(keyPair, hashTransaction(transaction));
	}

	/**
	 * Cosigns a Symbol transaction, producing a detached cosignature suitable for network propagation.
	 *
	 * @param keyPair Key pair of the cosignatory.
	 * @param transaction Transaction object.
	 * @return Signed detached cosignature.
	 */
	public DetachedCosignature cosignTransactionDetached(final KeyPair keyPair, final Transaction transaction) {
		return cosignTransactionHashDetached(keyPair, hashTransaction(transaction));
	}

	/**
	 * Hashes embedded transactions of an aggregate transaction.
	 *
	 * @param embeddedTransactions Embedded transactions to hash.
	 * @return Aggregate transactions hash.
	 */
	public static CryptoTypes.Hash256 hashEmbeddedTransactions(final List<? extends EmbeddedTransaction> embeddedTransactions) {
		final Merkle.MerkleHashBuilder hashBuilder = new Merkle.MerkleHashBuilder();
		for (EmbeddedTransaction embeddedTransaction : embeddedTransactions)
			hashBuilder.update(new CryptoTypes.Hash256(Transforms.sha3_256(embeddedTransaction.serialize())));

		return hashBuilder.finalHash();
	}

	/**
	 * Creates a network-compatible BIP32 path for the specified account.
	 *
	 * @param accountId Id of the account for which to generate a BIP32 path.
	 * @return BIP32 path for the specified account.
	 */
	public int[] bip32Path(final int accountId) {
		final int coinType = "mainnet".equals(network.name) ? 4343 : 1;
		return new int[]{
				44, coinType, accountId, 0, 0
		};
	}

	/**
	 * Derives a Symbol {@link KeyPair} from a BIP32 node.
	 *
	 * @param bip32Node BIP32 node.
	 * @return Derived key pair.
	 */
	public static KeyPair bip32NodeToKeyPair(final Bip32.Bip32Node bip32Node) {
		return new KeyPair(new CryptoTypes.PrivateKey(bip32Node.privateKey.bytes()));
	}

	// region helpers

	private static byte[] transactionDataBuffer(final TransactionType type, final byte[] transactionBuffer) {
		final int dataStart = TRANSACTION_HEADER_SIZE;
		int dataEnd = transactionBuffer.length;
		if (TransactionType.AGGREGATE_BONDED == type || TransactionType.AGGREGATE_COMPLETE == type) {
			final int version = transactionBuffer[TRANSACTION_HEADER_SIZE] & 0xFF;
			dataEnd = TRANSACTION_HEADER_SIZE + (3 <= version ? AGGREGATE_HASHED_SIZE : PRE_V3_AGGREGATE_HASHED_SIZE);
		}

		return java.util.Arrays.copyOfRange(transactionBuffer, dataStart, dataEnd);
	}

	// endregion

	// region account wrappers

	/**
	 * Symbol public account — known public key + derived address. Created via
	 * {@link SymbolFacade#createPublicAccount(CryptoTypes.PublicKey)}.
	 */
	public static class SymbolPublicAccount implements PublicAccount {
		/** Owning facade. */
		protected final SymbolFacade facade;

		/** Account public key. */
		private final CryptoTypes.PublicKey publicKey;

		/** Account address. */
		private final Address address;

		SymbolPublicAccount(final SymbolFacade facade, final CryptoTypes.PublicKey publicKey) {
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
	 * Symbol account — adds the private key pair to {@link SymbolPublicAccount}. Can sign and cosign transactions, and produce a
	 * {@link MessageEncoder}.
	 */
	public static final class SymbolAccount extends SymbolPublicAccount implements Account<Transaction, KeyPair> {
		/** Account key pair. */
		private final KeyPair keyPair;

		SymbolAccount(final SymbolFacade facade, final KeyPair keyPair) {
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
		 * Signs a Symbol transaction.
		 *
		 * @param transaction Transaction object.
		 * @return Transaction signature.
		 */
		public CryptoTypes.Signature signTransaction(final Transaction transaction) {
			return this.facade.signTransaction(this.keyPair, transaction);
		}

		/**
		 * Cosigns a Symbol transaction, producing a cosignature suitable for attaching to an aggregate.
		 *
		 * @param transaction Transaction object.
		 * @return Signed cosignature.
		 */
		public Cosignature cosignTransaction(final Transaction transaction) {
			return this.facade.cosignTransaction(this.keyPair, transaction);
		}

		/**
		 * Cosigns a Symbol transaction, producing a detached cosignature suitable for network propagation.
		 *
		 * @param transaction Transaction object.
		 * @return Signed detached cosignature.
		 */
		public DetachedCosignature cosignTransactionDetached(final Transaction transaction) {
			return this.facade.cosignTransactionDetached(this.keyPair, transaction);
		}

		/**
		 * Cosigns a Symbol transaction hash, producing a cosignature suitable for attaching to an aggregate.
		 *
		 * @param transactionHash Transaction hash.
		 * @return Signed cosignature.
		 */
		public Cosignature cosignTransactionHash(final CryptoTypes.Hash256 transactionHash) {
			return SymbolFacade.cosignTransactionHash(this.keyPair, transactionHash);
		}

		/**
		 * Cosigns a Symbol transaction hash, producing a detached cosignature suitable for network propagation.
		 *
		 * @param transactionHash Transaction hash.
		 * @return Signed detached cosignature.
		 */
		public DetachedCosignature cosignTransactionHashDetached(final CryptoTypes.Hash256 transactionHash) {
			return SymbolFacade.cosignTransactionHashDetached(this.keyPair, transactionHash);
		}
	}

	// endregion
}
