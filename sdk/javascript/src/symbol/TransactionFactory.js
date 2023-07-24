import {
	Address,
	/* eslint-disable no-unused-vars */
	Network
	/* eslint-enable no-unused-vars */
} from './Network.js';
import { generateMosaicId, generateNamespaceId } from './idGenerator.js';
import * as sc from './models.js';
import {
	Hash256,
	PublicKey,
	/* eslint-disable no-unused-vars */
	Signature
	/* eslint-enable no-unused-vars */
} from '../CryptoTypes.js';
import RuleBasedTransactionFactory from '../RuleBasedTransactionFactory.js';
import { uint8ToHex } from '../utils/converter.js';

/**
 * Factory for creating Symbol transactions.
 */
export default class TransactionFactory {
	/**
	 * Creates a factory for the specified network.
	 * @param {Network} network Symbol network.
	 * @param {Map<string, function>|undefined} typeRuleOverrides Type rule overrides.
	 */
	constructor(network, typeRuleOverrides = undefined) {
		/**
		 * @private
		 */
		this._factory = TransactionFactory._buildRules(typeRuleOverrides); // eslint-disable-line no-underscore-dangle

		/**
		 * @private
		 */
		this._network = network;
	}

	/**
	 * Gets rule names with registered hints.
	 */
	get ruleNames() {
		return Array.from(this._factory.rules.keys());
	}

	/**
	 * Looks up the friendly name for the specified transaction.
	 * @param {sc.TransactionType} transactionType Transaction type.
	 * @param {number} transactionVersion Transaction version.
	 * @returns {string} Transaction friendly name.
	 */
	static lookupTransactionName(transactionType, transactionVersion) {
		return `${sc.TransactionType.valueToKey(transactionType.value).toLowerCase()}_transaction_v${transactionVersion}`;
	}

	/**
	 * Creates a transaction from a transaction descriptor.
	 * @template TTransaction
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @param {boolean} autosort When set (default), descriptor arrays requiring ordering will be automatically sorted.
	 *                           When unset, descriptor arrays will be presumed to be already sorted.
	 * @param {{createByName: function}} FactoryClass Factory class used to create the transaction.
	 * @returns {TTransaction} Newly created transaction.
	 * @private
	 */
	_createAndExtend(transactionDescriptor, autosort, FactoryClass) {
		const transaction = this._factory.createFromFactory(FactoryClass.createByName, {
			...transactionDescriptor,
			network: this._network.identifier
		});
		if (autosort)
			transaction.sort();

		// autogenerate artifact ids
		if (sc.TransactionType.NAMESPACE_REGISTRATION === transaction.type) {
			const parentId = sc.NamespaceRegistrationType.CHILD === transaction.registrationType ? transaction.parentId.value : 0n;
			const rawNamespaceId = generateNamespaceId(new TextDecoder().decode(transaction.name), parentId);
			transaction.id = new sc.NamespaceId(rawNamespaceId);
		} else if (sc.TransactionType.MOSAIC_DEFINITION === transaction.type) {
			const address = this._network.publicKeyToAddress(new PublicKey(transaction.signerPublicKey.bytes));
			transaction.id = new sc.MosaicId(generateMosaicId(address, transaction.nonce.value));
		}

		return transaction;
	}

	/**
	 * Creates a transaction from a transaction descriptor.
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @param {boolean} autosort When set (default), descriptor arrays requiring ordering will be automatically sorted.
	 *                           When unset, descriptor arrays will be presumed to be already sorted.
	 * @returns {sc.Transaction} Newly created transaction.
	 */
	create(transactionDescriptor, autosort = true) {
		return this._createAndExtend(transactionDescriptor, autosort, sc.TransactionFactory);
	}

	/**
	 * Creates an embedded transaction from a transaction descriptor.
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @param {boolean} autosort When set (default), descriptor arrays requiring ordering will be automatically sorted.
	 *                           When unset, descriptor arrays will be presumed to be already sorted.
	 * @returns {sc.EmbeddedTransaction} Newly created transaction.
	 */
	createEmbedded(transactionDescriptor, autosort = true) {
		return this._createAndExtend(transactionDescriptor, autosort, sc.EmbeddedTransactionFactory);
	}

	/**
	 * Attaches a signature to a transaction.
	 * @param {sc.Transaction} transaction Transaction object.
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

	/**
	 * Tries to coerce an sdk type to a model type.
	 * @param {object} value Value to convert.
	 * @returns {sc.Address|undefined} Converted value or undefined.
	 * @private
	 */
	static _symbolTypeConverter(value) {
		if (value instanceof Address)
			return new sc.UnresolvedAddress(value.bytes);

		return undefined;
	}

	/**
	 * Builds a rule based transaction factory.
	 * @param {Map<string, function>|undefined} typeRuleOverrides Type rule overrides.
	 * @returns {RuleBasedTransactionFactory} Rule based transaction factory.
	 * @private
	 */
	static _buildRules(typeRuleOverrides) {
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
