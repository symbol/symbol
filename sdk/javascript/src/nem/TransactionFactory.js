import {
	Address,
	/* eslint-disable no-unused-vars */
	Network
	/* eslint-enable no-unused-vars */
} from './Network.js';
import * as nc from './models.js';
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
 * Factory for creating NEM transactions.
 */
export default class TransactionFactory {
	/**
	 * Creates a factory for the specified network.
	 * @param {Network} network NEM network.
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
	 * @param {nc.TransactionType} transactionType Transaction type.
	 * @param {number} transactionVersion Transaction version.
	 * @returns {string} Transaction friendly name.
	 */
	static lookupTransactionName(transactionType, transactionVersion) {
		return `${nc.TransactionType.valueToKey(transactionType.value).toLowerCase()}_transaction_v${transactionVersion}`;
	}

	/**
	 * Creates a transaction from a transaction descriptor.
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @param {boolean} autosort When set (default), descriptor arrays requiring ordering will be automatically sorted.
	 *                           When unset, descriptor arrays will be presumed to be already sorted.
	 * @returns {nc.Transaction} Newly created transaction.
	 */
	create(transactionDescriptor, autosort = true) {
		const transaction = this._factory.createFromFactory(nc.TransactionFactory.createByName, {
			...transactionDescriptor,
			network: this._network.identifier
		});
		if (autosort)
			transaction.sort();

		// hack: explicitly translate transfer message
		if (nc.TransactionType.TRANSFER === transaction.type && transaction.message && 'string' === typeof (transaction.message.message))
			transaction.message.message = new TextEncoder().encode(transaction.message.message);

		return transaction;
	}

	/**
	 * Converts a transaction to a non-verifiable transaction.
	 * @param {nc.Transaction|nc.NonVerifiableTransaction} transaction Transaction object.
	 * @returns {nc.NonVerifiableTransaction} Non-verifiable transaction object.
	 */
	static toNonVerifiableTransaction(transaction) {
		let nonVerifiableClassName = transaction.constructor.name;
		if (0 !== nonVerifiableClassName.indexOf('NonVerifiable'))
			nonVerifiableClassName = `NonVerifiable${nonVerifiableClassName}`;

		const NonVerifiableClass = nc[nonVerifiableClassName];
		const nonVerifiableTransaction = new NonVerifiableClass();
		Object.getOwnPropertyNames(transaction).forEach(key => {
			if (key in nonVerifiableTransaction)
				nonVerifiableTransaction[key] = transaction[key];
		});

		return nonVerifiableTransaction;
	}

	/**
	 * Attaches a signature to a transaction.
	 * @param {nc.Transaction} transaction Transaction object.
	 * @param {Signature} signature Signature to attach.
	 * @returns {string} JSON transaction payload.
	 */
	static attachSignature(transaction, signature) {
		transaction.signature = new nc.Signature(signature.bytes);

		const transactionHex = uint8ToHex(this.toNonVerifiableTransaction(transaction).serialize());
		const signatureHex = signature.toString();
		const jsonPayload = `{"data":"${transactionHex}", "signature":"${signatureHex}"}`;
		return jsonPayload;
	}

	/**
	 * Tries to coerce an sdk type to a model type.
	 * @param {object} value Value to convert.
	 * @returns {nc.Address|undefined} Converted value or undefined.
	 * @private
	 */
	static _nemTypeConverter(value) {
		if (value instanceof Address) {
			// yes, unfortunately, nem's Address is 40 bytes string, but we need to pass it as actual bytes not to confuse ByteArray
			return new nc.Address(new TextEncoder().encode(value.toString()));
		}

		return undefined;
	}

	/**
	 * Builds a rule based transaction factory.
	 * @param {Map<string, function>|undefined} typeRuleOverrides Type rule overrides.
	 * @returns {RuleBasedTransactionFactory} Rule based transaction factory.
	 * @private
	 */
	static _buildRules(typeRuleOverrides) {
		const factory = new RuleBasedTransactionFactory(nc, this._nemTypeConverter, typeRuleOverrides);
		factory.autodetect();

		[
			'LinkAction', 'MessageType', 'MosaicSupplyChangeAction', 'MosaicTransferFeeType',
			'MultisigAccountModificationType', 'NetworkType', 'TransactionType'
		].forEach(name => { factory.addEnumParser(name); });

		[
			'Message', 'NamespaceId', 'MosaicId', 'Mosaic', 'SizePrefixedMosaic', 'MosaicLevy',
			'MosaicProperty', 'SizePrefixedMosaicProperty', 'MosaicDefinition',
			'MultisigAccountModification', 'SizePrefixedMultisigAccountModification'
		].forEach(name => { factory.addStructParser(name); });

		const sdkTypeMapping = {
			Address,
			Hash256,
			PublicKey
		};
		Object.keys(sdkTypeMapping).forEach(name => { factory.addPodParser(name, sdkTypeMapping[name]); });

		[
			'struct:SizePrefixedMosaic', 'struct:SizePrefixedMosaicProperty', 'struct:SizePrefixedMultisigAccountModification'
		].forEach(name => { factory.addArrayParser(name); });

		return factory;
	}
}
