const { Address } = require('./Network');
const nc = require('./models');
const { Hash256, PublicKey } = require('../CryptoTypes');
const { RuleBasedTransactionFactory } = require('../RuleBasedTransactionFactory');
const { uint8ToHex } = require('../utils/converter');

/**
 * Factory for creating NEM transactions.
 */
class TransactionFactory {
	/**
	 * Creates a factory for the specified network.
	 * @param {Network} network NEM network.
	 * @param {Map} typeRuleOverrides Type rule overrides.
	 */
	constructor(network, typeRuleOverrides) {
		this.factory = TransactionFactory.buildRules(typeRuleOverrides);
		this.network = network;
	}

	/**
	 * Creates a transaction from a transaction descriptor.
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @returns {object} Newly created transaction.
	 */
	create(transactionDescriptor) {
		const transaction = this.factory.createFromFactory(nc.TransactionFactory.createByName, {
			...transactionDescriptor,
			network: this.network.identifier
		});

		// hack: explicitly translate transfer message
		if (nc.TransactionType.TRANSFER === transaction.type && 'string' === typeof (transaction.message.message))
			transaction.message.message = new TextEncoder().encode(transaction.message.message);

		return transaction;
	}

	/**
	 * Converts a transaction to a non-verifiable transaction.
	 * @param {object} transaction Transaction object.
	 * @returns {object} Non-verifiable transaction object.
	 */
	static toNonVerifiableTransaction(transaction) {
		let nonVerifiableClassName = transaction.constructor.name;
		if (0 !== nonVerifiableClassName.indexOf('NonVerifiable'))
			nonVerifiableClassName = `NonVerifiable${nonVerifiableClassName}`;

		const NonVerifiableClass = nc[nonVerifiableClassName];
		const nonVerifiableTransaction = new NonVerifiableClass();
		Object.getOwnPropertyNames(transaction).forEach(key => {
			nonVerifiableTransaction[key] = transaction[key];
		});

		return nonVerifiableTransaction;
	}

	/**
	 * Attaches a signature to a transaction.
	 * @param {object} transaction Transaction object.
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

	static _nemTypeConverter(value) {
		if (value instanceof Address) {
			// yes, unfortunately, nem's Address is 40 bytes string, but we need to pass it as actual bytes not to confuse ByteArray
			return new nc.Address(new TextEncoder().encode(value.toString()));
		}

		return undefined;
	}

	static buildRules(typeRuleOverrides) {
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

module.exports = { TransactionFactory };
