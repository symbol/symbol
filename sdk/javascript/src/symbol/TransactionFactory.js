const { Address } = require('./Network');
const { generateNamespaceId, generateMosaicId } = require('./idGenerator');
const sc = require('./models');
const { Hash256, PublicKey } = require('../CryptoTypes');
const { RuleBasedTransactionFactory } = require('../RuleBasedTransactionFactory');
const { uint8ToHex } = require('../utils/converter');

/**
 * Factory for creating Symbol transactions.
 */
class TransactionFactory {
	/**
	 * Creates a factory for the specified network.
	 * @param {Network} network Symbol network.
	 * @param {Map} typeRuleOverrides Type rule overrides.
	 */
	constructor(network, typeRuleOverrides) {
		this.factory = TransactionFactory.buildRules(typeRuleOverrides);
		this.network = network;
	}

	_createAndExtend(transactionDescriptor, FactoryClass) {
		const transaction = this.factory.createFromFactory(FactoryClass.createByName, {
			...transactionDescriptor,
			network: this.network.identifier
		});

		// autogenerate artifact ids
		if (sc.TransactionType.NAMESPACE_REGISTRATION === transaction.type) {
			const rawNamespaceId = generateNamespaceId(new TextDecoder().decode(transaction.name), transaction.parentId.value);
			transaction.id = new sc.NamespaceId(rawNamespaceId);
		} else if (sc.TransactionType.MOSAIC_DEFINITION === transaction.type) {
			const address = this.network.publicKeyToAddress(new PublicKey(transaction.signerPublicKey.bytes));
			transaction.id = new sc.MosaicId(generateMosaicId(address, transaction.nonce.value));
		}

		return transaction;
	}

	/**
	 * Creates a transaction from a transaction descriptor.
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @returns {object} Newly created transaction.
	 */
	create(transactionDescriptor) {
		return this._createAndExtend(transactionDescriptor, sc.TransactionFactory);
	}

	/**
	 * Creates an embedded transaction from a transaction descriptor.
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @returns {object} Newly created transaction.
	 */
	createEmbedded(transactionDescriptor) {
		return this._createAndExtend(transactionDescriptor, sc.EmbeddedTransactionFactory);
	}

	/**
	 * Attaches a signature to a transaction.
	 * @param {object} transaction Transaction object.
	 * @param {Signature} signature Signature to attach.
	 * @returns {string} JSON transaction payload.
	 */
	static attachSignature(transaction, signature) {
		transaction.signature = new sc.Signature(signature.bytes);

		const transactionBuffer = transaction.serialize();
		const hexPayload = uint8ToHex(transactionBuffer);
		const jsonPayload = `{"payload": "${hexPayload}"}`;
		return jsonPayload;
	}

	static _symbolTypeConverter(value) {
		if (value instanceof Address)
			return new sc.UnresolvedAddress(value.bytes);

		return undefined;
	}

	static buildRules(typeRuleOverrides) {
		const factory = new RuleBasedTransactionFactory(sc, this._symbolTypeConverter, typeRuleOverrides);
		factory.autodetect();

		['MosaicFlags', 'AccountRestrictionFlags'].forEach(name => { factory.addFlagsParser(name); });

		[
			'AliasAction', 'LinkAction', 'LockHashAlgorithm',
			'MosaicRestrictionType', 'MosaicSupplyChangeAction',
			'NamespaceRegistrationType', 'NetworkType', 'TransactionType'
		].forEach(name => { factory.addEnumParser(name); });

		factory.addStructParser('UnresolvedMosaic');

		const sdkTypeMapping = {
			UnresolvedAddress: Address,
			Address,
			Hash256,
			PublicKey,
			VotingPublicKey: PublicKey
		};
		Object.keys(sdkTypeMapping).forEach(name => { factory.addPodParser(name, sdkTypeMapping[name]); });

		['UnresolvedMosaicId', 'TransactionType', 'UnresolvedAddress', 'struct:UnresolvedMosaic'].forEach(name => {
			factory.addArrayParser(name);
		});

		return factory;
	}
}

module.exports = { TransactionFactory };
