/* eslint-disable max-len */

import { Address } from './Network.js';
import * as models from './models.js';
import { Hash256, PublicKey } from '../CryptoTypes.js';

/**
 * Type safe descriptor used to generate a descriptor map for MosaicDescriptor.
 *
 * A quantity of a certain mosaic.
 */
export class MosaicDescriptor {
	/**
	 * Creates a descriptor for Mosaic.
	 * @param {models.MosaicId} mosaicId Mosaic identifier.
	 * @param {models.Amount} amount Mosaic amount.
	 */
	constructor(mosaicId, amount) {
		this.rawDescriptor = {
			mosaicId,
			amount
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for UnresolvedMosaicDescriptor.
 *
 * A quantity of a certain mosaic, specified either through a MosaicId or an alias.
 */
export class UnresolvedMosaicDescriptor {
	/**
	 * Creates a descriptor for UnresolvedMosaic.
	 * @param {models.UnresolvedMosaicId} mosaicId Unresolved mosaic identifier.
	 * @param {models.Amount} amount Mosaic amount.
	 */
	constructor(mosaicId, amount) {
		this.rawDescriptor = {
			mosaicId,
			amount
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AccountKeyLinkTransactionV1Descriptor.
 *
 * This transaction is required for all accounts wanting to activate remote or delegated harvesting (V1, latest).
 * Announce an AccountKeyLinkTransaction to delegate the account importance score to a proxy account.
 */
export class AccountKeyLinkTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AccountKeyLinkTransactionV1.
	 * @param {PublicKey} linkedPublicKey Linked public key.
	 * @param {models.LinkAction} linkAction Account link action.
	 */
	constructor(linkedPublicKey, linkAction) {
		this.rawDescriptor = {
			type: 'account_key_link_transaction_v1',
			linkedPublicKey,
			linkAction
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for NodeKeyLinkTransactionV1Descriptor.
 *
 * This transaction is required for all accounts willing to activate delegated harvesting (V1, latest).
 * Announce a NodeKeyLinkTransaction to link an account with a public key used by TLS to create sessions.
 */
export class NodeKeyLinkTransactionV1Descriptor {
	/**
	 * Creates a descriptor for NodeKeyLinkTransactionV1.
	 * @param {PublicKey} linkedPublicKey Linked public key.
	 * @param {models.LinkAction} linkAction Account link action.
	 */
	constructor(linkedPublicKey, linkAction) {
		this.rawDescriptor = {
			type: 'node_key_link_transaction_v1',
			linkedPublicKey,
			linkAction
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for CosignatureDescriptor.
 *
 * Cosignature attached to an AggregateCompleteTransaction or AggregateBondedTransaction.
 */
export class CosignatureDescriptor {
	/**
	 * Creates a descriptor for Cosignature.
	 * @param {bigint} version Version.
	 * @param {PublicKey} signerPublicKey Cosigner public key.
	 * @param {models.Signature} signature Transaction signature.
	 */
	constructor(version, signerPublicKey, signature) {
		this.rawDescriptor = {
			version,
			signerPublicKey,
			signature
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for DetachedCosignatureDescriptor.
 *
 * Cosignature detached from an AggregateCompleteTransaction or AggregateBondedTransaction.
 */
export class DetachedCosignatureDescriptor {
	/**
	 * Creates a descriptor for DetachedCosignature.
	 * @param {bigint} version Version.
	 * @param {PublicKey} signerPublicKey Cosigner public key.
	 * @param {models.Signature} signature Transaction signature.
	 * @param {Hash256} parentHash Hash of the AggregateBondedTransaction that is signed by this cosignature.
	 */
	constructor(version, signerPublicKey, signature, parentHash) {
		this.rawDescriptor = {
			version,
			signerPublicKey,
			signature,
			parentHash
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AggregateCompleteTransactionV1Descriptor.
 *
 * Send transactions in batches to different accounts (V1, deprecated).
 * Use this transaction when all required signatures are available when the transaction is created.
 */
export class AggregateCompleteTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AggregateCompleteTransactionV1.
	 * @param {Hash256} transactionsHash Hash of the aggregate's transaction.
	 * @param {models.EmbeddedTransaction[]|undefined} transactions Embedded transaction data.
	Transactions are variable-sized and the total payload size is in bytes.
	Embedded transactions cannot be aggregates.
	 * @param {models.Cosignature[]|undefined} cosignatures Cosignatures data.
	Fills up remaining body space after transactions.
	 */
	constructor(transactionsHash, transactions = undefined, cosignatures = undefined) {
		this.rawDescriptor = {
			type: 'aggregate_complete_transaction_v1',
			transactionsHash
		};

		if (transactions)
			this.rawDescriptor.transactions = transactions;

		if (cosignatures)
			this.rawDescriptor.cosignatures = cosignatures;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AggregateCompleteTransactionV2Descriptor.
 *
 * Send transactions in batches to different accounts (V2, deprecated).
 * Use this transaction when all required signatures are available when the transaction is created.
 */
export class AggregateCompleteTransactionV2Descriptor {
	/**
	 * Creates a descriptor for AggregateCompleteTransactionV2.
	 * @param {Hash256} transactionsHash Hash of the aggregate's transaction.
	 * @param {models.EmbeddedTransaction[]|undefined} transactions Embedded transaction data.
	Transactions are variable-sized and the total payload size is in bytes.
	Embedded transactions cannot be aggregates.
	 * @param {models.Cosignature[]|undefined} cosignatures Cosignatures data.
	Fills up remaining body space after transactions.
	 */
	constructor(transactionsHash, transactions = undefined, cosignatures = undefined) {
		this.rawDescriptor = {
			type: 'aggregate_complete_transaction_v2',
			transactionsHash
		};

		if (transactions)
			this.rawDescriptor.transactions = transactions;

		if (cosignatures)
			this.rawDescriptor.cosignatures = cosignatures;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AggregateCompleteTransactionV3Descriptor.
 *
 * Send transactions in batches to different accounts (V3, latest).
 * Use this transaction when all required signatures are available when the transaction is created.
 */
export class AggregateCompleteTransactionV3Descriptor {
	/**
	 * Creates a descriptor for AggregateCompleteTransactionV3.
	 * @param {Hash256} transactionsHash Hash of the aggregate's transaction.
	 * @param {models.EmbeddedTransaction[]|undefined} transactions Embedded transaction data.
	Transactions are variable-sized and the total payload size is in bytes.
	Embedded transactions cannot be aggregates.
	 * @param {models.Cosignature[]|undefined} cosignatures Cosignatures data.
	Fills up remaining body space after transactions.
	 */
	constructor(transactionsHash, transactions = undefined, cosignatures = undefined) {
		this.rawDescriptor = {
			type: 'aggregate_complete_transaction_v3',
			transactionsHash
		};

		if (transactions)
			this.rawDescriptor.transactions = transactions;

		if (cosignatures)
			this.rawDescriptor.cosignatures = cosignatures;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AggregateBondedTransactionV1Descriptor.
 *
 * Propose an arrangement of transactions between different accounts (V1, deprecated).
 * Use this transaction when not all required signatures are available when the transaction is created.
 * Missing signatures must be provided using a Cosignature or DetachedCosignature.
 * To prevent spam attacks, before trying to announce this transaction a HashLockTransaction must be successfully announced and confirmed.
 */
export class AggregateBondedTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AggregateBondedTransactionV1.
	 * @param {Hash256} transactionsHash Hash of the aggregate's transaction.
	 * @param {models.EmbeddedTransaction[]|undefined} transactions Embedded transaction data.
	Transactions are variable-sized and the total payload size is in bytes.
	Embedded transactions cannot be aggregates.
	 * @param {models.Cosignature[]|undefined} cosignatures Cosignatures data.
	Fills up remaining body space after transactions.
	 */
	constructor(transactionsHash, transactions = undefined, cosignatures = undefined) {
		this.rawDescriptor = {
			type: 'aggregate_bonded_transaction_v1',
			transactionsHash
		};

		if (transactions)
			this.rawDescriptor.transactions = transactions;

		if (cosignatures)
			this.rawDescriptor.cosignatures = cosignatures;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AggregateBondedTransactionV2Descriptor.
 *
 * Propose an arrangement of transactions between different accounts (V2, deprecated).
 * Use this transaction when not all required signatures are available when the transaction is created.
 * Missing signatures must be provided using a Cosignature or DetachedCosignature.
 * To prevent spam attacks, before trying to announce this transaction a HashLockTransaction must be successfully announced and confirmed.
 */
export class AggregateBondedTransactionV2Descriptor {
	/**
	 * Creates a descriptor for AggregateBondedTransactionV2.
	 * @param {Hash256} transactionsHash Hash of the aggregate's transaction.
	 * @param {models.EmbeddedTransaction[]|undefined} transactions Embedded transaction data.
	Transactions are variable-sized and the total payload size is in bytes.
	Embedded transactions cannot be aggregates.
	 * @param {models.Cosignature[]|undefined} cosignatures Cosignatures data.
	Fills up remaining body space after transactions.
	 */
	constructor(transactionsHash, transactions = undefined, cosignatures = undefined) {
		this.rawDescriptor = {
			type: 'aggregate_bonded_transaction_v2',
			transactionsHash
		};

		if (transactions)
			this.rawDescriptor.transactions = transactions;

		if (cosignatures)
			this.rawDescriptor.cosignatures = cosignatures;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AggregateBondedTransactionV3Descriptor.
 *
 * Propose an arrangement of transactions between different accounts (V3, latest).
 * Use this transaction when not all required signatures are available when the transaction is created.
 * Missing signatures must be provided using a Cosignature or DetachedCosignature.
 * To prevent spam attacks, before trying to announce this transaction a HashLockTransaction must be successfully announced and confirmed.
 */
export class AggregateBondedTransactionV3Descriptor {
	/**
	 * Creates a descriptor for AggregateBondedTransactionV3.
	 * @param {Hash256} transactionsHash Hash of the aggregate's transaction.
	 * @param {models.EmbeddedTransaction[]|undefined} transactions Embedded transaction data.
	Transactions are variable-sized and the total payload size is in bytes.
	Embedded transactions cannot be aggregates.
	 * @param {models.Cosignature[]|undefined} cosignatures Cosignatures data.
	Fills up remaining body space after transactions.
	 */
	constructor(transactionsHash, transactions = undefined, cosignatures = undefined) {
		this.rawDescriptor = {
			type: 'aggregate_bonded_transaction_v3',
			transactionsHash
		};

		if (transactions)
			this.rawDescriptor.transactions = transactions;

		if (cosignatures)
			this.rawDescriptor.cosignatures = cosignatures;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for VotingKeyLinkTransactionV1Descriptor.
 *
 * Link an account with a public key required for finalization voting (V1, latest).
 * This transaction is required for node operators wanting to vote for [finalization](/concepts/block.html#finalization).
 * Announce a VotingKeyLinkTransaction to associate a voting key with an account during a fixed period. An account can be linked to up to **3** different voting keys at the same time.
 * The recommended production setting is to always have at least **2** linked keys with different ``endPoint`` values to ensure a key is registered after the first one expires.
 * See more details in [the manual node setup guide](/guides/network/running-a-symbol-node-manually.html#manual-voting-key-renewal).
 */
export class VotingKeyLinkTransactionV1Descriptor {
	/**
	 * Creates a descriptor for VotingKeyLinkTransactionV1.
	 * @param {PublicKey} linkedPublicKey Linked voting public key.
	 * @param {models.FinalizationEpoch} startEpoch Starting finalization epoch.
	 * @param {models.FinalizationEpoch} endEpoch Ending finalization epoch.
	 * @param {models.LinkAction} linkAction Account link action.
	 */
	constructor(linkedPublicKey, startEpoch, endEpoch, linkAction) {
		this.rawDescriptor = {
			type: 'voting_key_link_transaction_v1',
			linkedPublicKey,
			startEpoch,
			endEpoch,
			linkAction
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for VrfKeyLinkTransactionV1Descriptor.
 *
 * Link an account with a VRF public key required for harvesting (V1, latest).
 * Announce a VrfKeyLinkTransaction to link an account with a VRF public key. The linked key is used to randomize block production and leader/participant selection.
 * This transaction is required for all accounts wishing to [harvest](/concepts/harvesting.html).
 */
export class VrfKeyLinkTransactionV1Descriptor {
	/**
	 * Creates a descriptor for VrfKeyLinkTransactionV1.
	 * @param {PublicKey} linkedPublicKey Linked VRF public key.
	 * @param {models.LinkAction} linkAction Account link action.
	 */
	constructor(linkedPublicKey, linkAction) {
		this.rawDescriptor = {
			type: 'vrf_key_link_transaction_v1',
			linkedPublicKey,
			linkAction
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for HashLockTransactionV1Descriptor.
 *
 * Lock a deposit needed to announce an AggregateBondedTransaction (V1, latest).
 * An AggregateBondedTransaction consumes network resources as it is stored in every node's partial cache while it waits to be fully signed. To avoid spam attacks a HashLockTransaction must be announced and confirmed before an AggregateBondedTransaction can be announced. The HashLockTransaction locks a certain amount of funds (**10** XYM by default) until the aggregate is signed.
 * Upon completion of the aggregate, the locked funds become available again to the account that signed the HashLockTransaction.
 * If the lock expires before the aggregate is signed by all cosignatories (**48h by default), the locked funds become a reward collected by the block harvester at the height where the lock expires.
 * \note It is not necessary to sign the aggregate and its HashLockTransaction with the same account. For example, if Bob wants to announce an aggregate and does not have enough funds to announce a HashLockTransaction, he can ask Alice to announce the lock transaction for him by sharing the signed AggregateTransaction hash.
 */
export class HashLockTransactionV1Descriptor {
	/**
	 * Creates a descriptor for HashLockTransactionV1.
	 * @param {UnresolvedMosaicDescriptor} mosaic Locked mosaic.
	 * @param {models.BlockDuration} duration Number of blocks for which a lock should be valid.
	The default maximum is 48h (See the `maxHashLockDuration` network property).
	 * @param {Hash256} hash Hash of the AggregateBondedTransaction to be confirmed before unlocking the mosaics.
	 */
	constructor(mosaic, duration, hash) {
		this.rawDescriptor = {
			type: 'hash_lock_transaction_v1',
			mosaic: mosaic.toMap(),
			duration,
			hash
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for SecretLockTransactionV1Descriptor.
 *
 * Start a token swap between different chains (V1, latest).
 * Use a SecretLockTransaction to transfer mosaics between two accounts. The mosaics sent remain locked until a valid SecretProofTransaction unlocks them.
 * The default expiration date is **365 days** after announcement (See the `maxSecretLockDuration` network property). If the lock expires before a valid SecretProofTransaction is announced the locked amount goes back to the initiator of the SecretLockTransaction.
 */
export class SecretLockTransactionV1Descriptor {
	/**
	 * Creates a descriptor for SecretLockTransactionV1.
	 * @param {Address} recipientAddress Address that receives the funds once successfully unlocked by a SecretProofTransaction.
	 * @param {Hash256} secret Hashed proof.
	 * @param {UnresolvedMosaicDescriptor} mosaic Locked mosaics.
	 * @param {models.BlockDuration} duration Number of blocks to wait for the SecretProofTransaction.
	 * @param {models.LockHashAlgorithm} hashAlgorithm Algorithm used to hash the proof.
	 */
	constructor(recipientAddress, secret, mosaic, duration, hashAlgorithm) {
		this.rawDescriptor = {
			type: 'secret_lock_transaction_v1',
			recipientAddress,
			secret,
			mosaic: mosaic.toMap(),
			duration,
			hashAlgorithm
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for SecretProofTransactionV1Descriptor.
 *
 * Conclude a token swap between different chains (V1, latest).
 * Use a SecretProofTransaction to unlock the funds locked by a SecretLockTransaction.
 * The transaction must prove knowing the *proof* that unlocks the mosaics.
 */
export class SecretProofTransactionV1Descriptor {
	/**
	 * Creates a descriptor for SecretProofTransactionV1.
	 * @param {Address} recipientAddress Address that receives the funds once unlocked.
	 * @param {Hash256} secret Hashed proof.
	 * @param {models.LockHashAlgorithm} hashAlgorithm Algorithm used to hash the proof.
	 * @param {Uint8Array|string|undefined} proof Original random set of bytes that were hashed.
	 */
	constructor(recipientAddress, secret, hashAlgorithm, proof = undefined) {
		this.rawDescriptor = {
			type: 'secret_proof_transaction_v1',
			recipientAddress,
			secret,
			hashAlgorithm
		};

		if (proof)
			this.rawDescriptor.proof = proof;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AccountMetadataTransactionV1Descriptor.
 *
 * Associate a key-value state ([metadata](/concepts/metadata.html)) to an **account** (V1, latest).
 * \note This transaction must **always** be wrapped in an AggregateTransaction so that a cosignature from `target_address` can be provided. Without this cosignature the transaction is invalid.
 * Compare to MosaicMetadataTransaction and NamespaceMetadataTransaction.
 */
export class AccountMetadataTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AccountMetadataTransactionV1.
	 * @param {Address} targetAddress Account whose metadata should be modified.
	 * @param {bigint} scopedMetadataKey Metadata key scoped to source, target and type.
	 * @param {number} valueSizeDelta Change in value size in bytes, compared to previous size.
	 * @param {Uint8Array|string|undefined} value Difference between existing value and new value. \note When there is no existing value, this array is directly used and `value_size_delta`==`value_size`. \note When there is an existing value, the new value is the byte-wise XOR of the previous value and this array.
	 */
	constructor(targetAddress, scopedMetadataKey, valueSizeDelta, value = undefined) {
		this.rawDescriptor = {
			type: 'account_metadata_transaction_v1',
			targetAddress,
			scopedMetadataKey,
			valueSizeDelta
		};

		if (value)
			this.rawDescriptor.value = value;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicMetadataTransactionV1Descriptor.
 *
 * Associate a key-value state ([metadata](/concepts/metadata.html)) to a **mosaic** (V1, latest).
 * Compare to AccountMetadataTransaction and NamespaceMetadataTransaction.
 */
export class MosaicMetadataTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicMetadataTransactionV1.
	 * @param {Address} targetAddress Account owning the mosaic whose metadata should be modified.
	 * @param {bigint} scopedMetadataKey Metadata key scoped to source, target and type.
	 * @param {models.UnresolvedMosaicId} targetMosaicId Mosaic whose metadata should be modified.
	 * @param {number} valueSizeDelta Change in value size in bytes, compared to previous size.
	 * @param {Uint8Array|string|undefined} value Difference between existing value and new value. \note When there is no existing value, this array is directly used and `value_size_delta`==`value_size`. \note When there is an existing value, the new value is the byte-wise XOR of the previous value and this array.
	 */
	constructor(targetAddress, scopedMetadataKey, targetMosaicId, valueSizeDelta, value = undefined) {
		this.rawDescriptor = {
			type: 'mosaic_metadata_transaction_v1',
			targetAddress,
			scopedMetadataKey,
			targetMosaicId,
			valueSizeDelta
		};

		if (value)
			this.rawDescriptor.value = value;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for NamespaceMetadataTransactionV1Descriptor.
 *
 * Associate a key-value state ([metadata](/concepts/metadata.html)) to a **namespace** (V1, latest).
 * Compare to AccountMetadataTransaction and MosaicMetadataTransaction.
 */
export class NamespaceMetadataTransactionV1Descriptor {
	/**
	 * Creates a descriptor for NamespaceMetadataTransactionV1.
	 * @param {Address} targetAddress Account owning the namespace whose metadata should be modified.
	 * @param {bigint} scopedMetadataKey Metadata key scoped to source, target and type.
	 * @param {models.NamespaceId} targetNamespaceId Namespace whose metadata should be modified.
	 * @param {number} valueSizeDelta Change in value size in bytes, compared to previous size.
	 * @param {Uint8Array|string|undefined} value Difference between existing value and new value. \note When there is no existing value, this array is directly used and `value_size_delta`==`value_size`. \note When there is an existing value, the new value is the byte-wise XOR of the previous value and this array.
	 */
	constructor(targetAddress, scopedMetadataKey, targetNamespaceId, valueSizeDelta, value = undefined) {
		this.rawDescriptor = {
			type: 'namespace_metadata_transaction_v1',
			targetAddress,
			scopedMetadataKey,
			targetNamespaceId,
			valueSizeDelta
		};

		if (value)
			this.rawDescriptor.value = value;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicDefinitionTransactionV1Descriptor.
 *
 * Create a new  [mosaic](/concepts/mosaic.html) (V1, latest).
 */
export class MosaicDefinitionTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicDefinitionTransactionV1.
	 * @param {models.MosaicId} id Unique mosaic identifier obtained from the generator account's public key and the `nonce`.
	The SDK's can take care of generating this ID for you.
	 * @param {models.BlockDuration} duration Mosaic duration expressed in blocks. If set to 0, the mosaic never expires.
	 * @param {models.MosaicNonce} nonce Random nonce used to generate the mosaic id.
	 * @param {models.MosaicFlags} flags Mosaic flags.
	 * @param {number} divisibility Mosaic divisibility.
	 */
	constructor(id, duration, nonce, flags, divisibility) {
		this.rawDescriptor = {
			type: 'mosaic_definition_transaction_v1',
			id,
			duration,
			nonce,
			flags,
			divisibility
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicSupplyChangeTransactionV1Descriptor.
 *
 * Change the total supply of a mosaic (V1, latest).
 */
export class MosaicSupplyChangeTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicSupplyChangeTransactionV1.
	 * @param {models.UnresolvedMosaicId} mosaicId Affected mosaic identifier.
	 * @param {models.Amount} delta Change amount. It cannot be negative, use the `action` field to indicate if this amount should be **added** or **subtracted** from the current supply.
	 * @param {models.MosaicSupplyChangeAction} action Supply change action.
	 */
	constructor(mosaicId, delta, action) {
		this.rawDescriptor = {
			type: 'mosaic_supply_change_transaction_v1',
			mosaicId,
			delta,
			action
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicSupplyRevocationTransactionV1Descriptor.
 *
 * Revoke mosaic (V1, latest).
 */
export class MosaicSupplyRevocationTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicSupplyRevocationTransactionV1.
	 * @param {Address} sourceAddress Address from which tokens should be revoked.
	 * @param {UnresolvedMosaicDescriptor} mosaic Revoked mosaic and amount.
	 */
	constructor(sourceAddress, mosaic) {
		this.rawDescriptor = {
			type: 'mosaic_supply_revocation_transaction_v1',
			sourceAddress,
			mosaic: mosaic.toMap()
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MultisigAccountModificationTransactionV1Descriptor.
 *
 * Create or modify a [multi-signature](/concepts/multisig-account.html) account (V1, latest).
 * This transaction allows you to: - Transform a regular account into a multisig account. - Change the configurable properties of a multisig account. - Add or delete cosignatories from a multisig account (removing all cosignatories turns a multisig account into a regular account again).
 */
export class MultisigAccountModificationTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MultisigAccountModificationTransactionV1.
	 * @param {number} minRemovalDelta Relative change to the **minimum** number of cosignatures required when **removing a cosignatory**.
	E.g., when moving from 0 to 2 cosignatures this number would be **2**. When moving from 4 to 3 cosignatures, the number would be **-1**.
	 * @param {number} minApprovalDelta Relative change to the **minimum** number of cosignatures required when **approving a transaction**.
	E.g., when moving from 0 to 2 cosignatures this number would be **2**. When moving from 4 to 3 cosignatures, the number would be **-1**.
	 * @param {Address[]|undefined} addressAdditions Cosignatory address additions.
	All accounts in this list will be able to cosign transactions on behalf of the multisig account. The number of required cosignatures depends on the configured minimum approval and minimum removal values.
	 * @param {Address[]|undefined} addressDeletions Cosignatory address deletions.
	All accounts in this list will stop being able to cosign transactions on behalf of the multisig account. A transaction containing **any** address in this array requires a number of cosignatures at least equal to the minimum removal value.
	 */
	constructor(minRemovalDelta, minApprovalDelta, addressAdditions = undefined, addressDeletions = undefined) {
		this.rawDescriptor = {
			type: 'multisig_account_modification_transaction_v1',
			minRemovalDelta,
			minApprovalDelta
		};

		if (addressAdditions)
			this.rawDescriptor.addressAdditions = addressAdditions;

		if (addressDeletions)
			this.rawDescriptor.addressDeletions = addressDeletions;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AddressAliasTransactionV1Descriptor.
 *
 * Attach or detach a [namespace](/concepts/namespace.html) (alias) to an account address (V1, latest).
 * A namespace can be assigned to any account present in the network (this is, an account which has received at least one transaction).
 */
export class AddressAliasTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AddressAliasTransactionV1.
	 * @param {models.NamespaceId} namespaceId Identifier of the namespace that will become (or stop being) an alias for the address.
	 * @param {Address} address Aliased address.
	 * @param {models.AliasAction} aliasAction Alias action.
	 */
	constructor(namespaceId, address, aliasAction) {
		this.rawDescriptor = {
			type: 'address_alias_transaction_v1',
			namespaceId,
			address,
			aliasAction
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicAliasTransactionV1Descriptor.
 *
 * Attach or detach a [namespace](/concepts/namespace.html) to a Mosaic.(V1, latest)
 * Setting an alias to a mosaic is only possible if the account announcing this transaction has also created the namespace and the mosaic involved.
 */
export class MosaicAliasTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicAliasTransactionV1.
	 * @param {models.NamespaceId} namespaceId Identifier of the namespace that will become (or stop being) an alias for the Mosaic.
	 * @param {models.MosaicId} mosaicId Aliased mosaic identifier.
	 * @param {models.AliasAction} aliasAction Alias action.
	 */
	constructor(namespaceId, mosaicId, aliasAction) {
		this.rawDescriptor = {
			type: 'mosaic_alias_transaction_v1',
			namespaceId,
			mosaicId,
			aliasAction
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for NamespaceRegistrationTransactionV1Descriptor.
 *
 * Register (or renew a registration for) a [namespace](/concepts/namespace.html) (V1, latest).
 * Namespaces help keep assets organized.
 */
export class NamespaceRegistrationTransactionV1Descriptor {
	/**
	 * Creates a descriptor for NamespaceRegistrationTransactionV1.
	 * @param {models.NamespaceId} id Namespace identifier.
	 * @param {models.NamespaceRegistrationType} registrationType Namespace registration type.
	 * @param {models.BlockDuration|undefined} duration Number of confirmed blocks you would like to rent the namespace for. Required for root namespaces.
	 * @param {models.NamespaceId|undefined} parentId Parent namespace identifier. Required for sub-namespaces.
	 * @param {Uint8Array|string|undefined} name Namespace name.
	 */
	constructor(id, registrationType, duration = undefined, parentId = undefined, name = undefined) {
		this.rawDescriptor = {
			type: 'namespace_registration_transaction_v1',
			id,
			registrationType
		};

		if (duration)
			this.rawDescriptor.duration = duration;

		if (parentId)
			this.rawDescriptor.parentId = parentId;

		if (name)
			this.rawDescriptor.name = name;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AccountAddressRestrictionTransactionV1Descriptor.
 *
 * Allow or block incoming and outgoing transactions for a given a set of addresses (V1, latest).
 */
export class AccountAddressRestrictionTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AccountAddressRestrictionTransactionV1.
	 * @param {models.AccountRestrictionFlags} restrictionFlags Type of restriction being applied to the listed addresses.
	 * @param {Address[]|undefined} restrictionAdditions Array of account addresses being added to the restricted list.
	 * @param {Address[]|undefined} restrictionDeletions Array of account addresses being removed from the restricted list.
	 */
	constructor(restrictionFlags, restrictionAdditions = undefined, restrictionDeletions = undefined) {
		this.rawDescriptor = {
			type: 'account_address_restriction_transaction_v1',
			restrictionFlags
		};

		if (restrictionAdditions)
			this.rawDescriptor.restrictionAdditions = restrictionAdditions;

		if (restrictionDeletions)
			this.rawDescriptor.restrictionDeletions = restrictionDeletions;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AccountMosaicRestrictionTransactionV1Descriptor.
 *
 * Allow or block incoming transactions containing a given set of mosaics (V1, latest).
 */
export class AccountMosaicRestrictionTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AccountMosaicRestrictionTransactionV1.
	 * @param {models.AccountRestrictionFlags} restrictionFlags Type of restriction being applied to the listed mosaics.
	 * @param {models.UnresolvedMosaicId[]|undefined} restrictionAdditions Array of mosaics being added to the restricted list.
	 * @param {models.UnresolvedMosaicId[]|undefined} restrictionDeletions Array of mosaics being removed from the restricted list.
	 */
	constructor(restrictionFlags, restrictionAdditions = undefined, restrictionDeletions = undefined) {
		this.rawDescriptor = {
			type: 'account_mosaic_restriction_transaction_v1',
			restrictionFlags
		};

		if (restrictionAdditions)
			this.rawDescriptor.restrictionAdditions = restrictionAdditions;

		if (restrictionDeletions)
			this.rawDescriptor.restrictionDeletions = restrictionDeletions;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for AccountOperationRestrictionTransactionV1Descriptor.
 *
 * Allow or block outgoing transactions depending on their transaction type (V1, latest).
 */
export class AccountOperationRestrictionTransactionV1Descriptor {
	/**
	 * Creates a descriptor for AccountOperationRestrictionTransactionV1.
	 * @param {models.AccountRestrictionFlags} restrictionFlags Type of restriction being applied to the listed transaction types.
	 * @param {models.TransactionType[]|undefined} restrictionAdditions Array of transaction types being added to the restricted list.
	 * @param {models.TransactionType[]|undefined} restrictionDeletions Array of transaction types being rtemoved from the restricted list.
	 */
	constructor(restrictionFlags, restrictionAdditions = undefined, restrictionDeletions = undefined) {
		this.rawDescriptor = {
			type: 'account_operation_restriction_transaction_v1',
			restrictionFlags
		};

		if (restrictionAdditions)
			this.rawDescriptor.restrictionAdditions = restrictionAdditions;

		if (restrictionDeletions)
			this.rawDescriptor.restrictionDeletions = restrictionDeletions;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicAddressRestrictionTransactionV1Descriptor.
 *
 * Set address specific rules to transfer a restrictable mosaic (V1, latest).
 */
export class MosaicAddressRestrictionTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicAddressRestrictionTransactionV1.
	 * @param {models.UnresolvedMosaicId} mosaicId Identifier of the mosaic to which the restriction applies.
	 * @param {bigint} restrictionKey Restriction key.
	 * @param {bigint} previousRestrictionValue Previous restriction value. Set `previousRestrictionValue` to `FFFFFFFFFFFFFFFF` if the target address does not have a previous restriction value for this mosaic id and restriction key.
	 * @param {bigint} newRestrictionValue New restriction value.
	 * @param {Address} targetAddress Address being restricted.
	 */
	constructor(mosaicId, restrictionKey, previousRestrictionValue, newRestrictionValue, targetAddress) {
		this.rawDescriptor = {
			type: 'mosaic_address_restriction_transaction_v1',
			mosaicId,
			restrictionKey,
			previousRestrictionValue,
			newRestrictionValue,
			targetAddress
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for MosaicGlobalRestrictionTransactionV1Descriptor.
 *
 * Set global rules to transfer a restrictable mosaic (V1, latest).
 */
export class MosaicGlobalRestrictionTransactionV1Descriptor {
	/**
	 * Creates a descriptor for MosaicGlobalRestrictionTransactionV1.
	 * @param {models.UnresolvedMosaicId} mosaicId Identifier of the mosaic being restricted. The mosaic creator must be the signer of the transaction.
	 * @param {models.UnresolvedMosaicId} referenceMosaicId Identifier of the mosaic providing the restriction key. The mosaic global restriction for the mosaic identifier depends on global restrictions set on the reference mosaic. Set `reference_mosaic_id` to **0** if the mosaic giving the restriction equals the `mosaic_id`.
	 * @param {bigint} restrictionKey Restriction key relative to the reference mosaic identifier.
	 * @param {bigint} previousRestrictionValue Previous restriction value.
	 * @param {bigint} newRestrictionValue New restriction value.
	 * @param {models.MosaicRestrictionType} previousRestrictionType Previous restriction type.
	 * @param {models.MosaicRestrictionType} newRestrictionType New restriction type.
	 */
	constructor(
		mosaicId,
		referenceMosaicId,
		restrictionKey,
		previousRestrictionValue,
		newRestrictionValue,
		previousRestrictionType,
		newRestrictionType
	) {
		this.rawDescriptor = {
			type: 'mosaic_global_restriction_transaction_v1',
			mosaicId,
			referenceMosaicId,
			restrictionKey,
			previousRestrictionValue,
			newRestrictionValue,
			previousRestrictionType,
			newRestrictionType
		};
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}

/**
 * Type safe descriptor used to generate a descriptor map for TransferTransactionV1Descriptor.
 *
 * Send mosaics and messages between two accounts (V1, latest).
 */
export class TransferTransactionV1Descriptor {
	/**
	 * Creates a descriptor for TransferTransactionV1.
	 * @param {Address} recipientAddress recipient address
	 * @param {UnresolvedMosaicDescriptor[]|undefined} mosaics attached mosaics
	 * @param {Uint8Array|string|undefined} message attached message
	 */
	constructor(recipientAddress, mosaics = undefined, message = undefined) {
		this.rawDescriptor = {
			type: 'transfer_transaction_v1',
			recipientAddress
		};

		if (mosaics)
			this.rawDescriptor.mosaics = mosaics.map(descriptor => descriptor.toMap());

		if (message)
			this.rawDescriptor.message = message;
	}

	/**
	 * Builds a representation of this descriptor that can be passed to a factory function.
	 * @returns {object} Descriptor that can be passed to a factory function.
	 */
	toMap() {
		return this.rawDescriptor;
	}
}
