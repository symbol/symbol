/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

import AccountIdentifier from '../openApi/model/AccountIdentifier.js';
import Amount from '../openApi/model/Amount.js';
import Currency from '../openApi/model/Currency.js';
import Operation from '../openApi/model/Operation.js';
import OperationIdentifier from '../openApi/model/OperationIdentifier.js';
import Transaction from '../openApi/model/Transaction.js';
import TransactionIdentifier from '../openApi/model/TransactionIdentifier.js';
import { PublicKey, utils } from 'symbol-sdk';
import { Network, models } from 'symbol-sdk/nem';

const hexStringToUtfString = hex => new TextDecoder().decode(utils.hexToUint8(hex));

// region convertTransactionSdkJsonToRestJson

/**
 * Converts a transaction SDK JSON model into a transaction REST JSON model.
 * @param {object} transactionJson Transaction SDK JSON model.
 * @returns {object} Transaction REST JSON model.
 */
export const convertTransactionSdkJsonToRestJson = transactionJson => {
	const convertMosaicIdSdkJsonToRestJson = mosaicId => ({
		namespaceId: hexStringToUtfString(mosaicId.namespaceId.name),
		name: hexStringToUtfString(mosaicId.name)
	});

	const renameProperty = (originalName, newName, transform = undefined) => {
		const value = transactionJson[originalName];
		transactionJson[newName] = transform ? transform(value) : value;
		delete transactionJson[originalName];
	};

	if (transactionJson.mosaics) {
		transactionJson.mosaics = transactionJson.mosaics.map(mosaic => mosaic.mosaic).map(mosaic => ({
			quantity: mosaic.amount,
			mosaicId: convertMosaicIdSdkJsonToRestJson(mosaic.mosaicId)
		}));
	}

	if (transactionJson.mosaicId)
		transactionJson.mosaicId = convertMosaicIdSdkJsonToRestJson(transactionJson.mosaicId);

	if (transactionJson.modifications)
		transactionJson.modifications = transactionJson.modifications.map(modification => modification.modification);

	if (transactionJson.signerPublicKey)
		renameProperty('signerPublicKey', 'signer');

	if (transactionJson.innerTransaction)
		renameProperty('innerTransaction', 'otherTrans', convertTransactionSdkJsonToRestJson);

	if (transactionJson.cosignatures)
		renameProperty('cosignatures', 'signatures', cosignatures => cosignatures.map(convertTransactionSdkJsonToRestJson));

	if (transactionJson.recipientAddress)
		renameProperty('recipientAddress', 'recipient', hexStringToUtfString);

	if (transactionJson.minApprovalDelta)
		renameProperty('minApprovalDelta', 'minCosignatories', minApprovalDelta => ({ relativeChange: minApprovalDelta }));

	if (transactionJson.mosaicDefinition) {
		transactionJson.mosaicDefinition.id = convertMosaicIdSdkJsonToRestJson(transactionJson.mosaicDefinition.id);

		transactionJson.mosaicDefinition.properties = transactionJson.mosaicDefinition.properties
			.map(property => property.property)
			.map(property => ({
				name: hexStringToUtfString(property.name),
				value: hexStringToUtfString(property.value)
			}));

		renameProperty('rentalFee', 'creationFee');
		renameProperty('rentalFeeSink', 'creationFeeSink');
	}

	return transactionJson;
};

// endregion

/**
 * Parses NEM models into rosetta operations.
 */
export class OperationParser {
	/**
	 * Creates a parser.
	 * @param {Network} network Symbol network.
	 * @param {object} options Parser options.
	 */
	constructor(network, options) {
		this.network = network;
		this.options = {
			includeFeeOperation: false,
			...options
		};
	}

	/**
	 * Creates a rosetta operation.
	 * @param {number} id Operation id.
	 * @param {string} type Operation type.
	 * @returns {Operation} Rosetta operation.
	 * @private
	 */
	createOperation(id, type) {
		const operation = new Operation(new OperationIdentifier(id), type);
		operation.status = this.options.operationStatus;
		return operation;
	}

	/**
	 * Converts a public key string into a rosetta account identifier.
	 * @param {string} publicKeyString Public key string.
	 * @returns {AccountIdentifier} Account identifier.
	 * @private
	 */
	publicKeyStringToAccountIdentifier(publicKeyString) {
		const address = this.network.publicKeyToAddress(new PublicKey(publicKeyString));
		return new AccountIdentifier(address.toString());
	}

	/**
	 * Creates a credit operation.
	 * @param {object} options Credit options.
	 * @returns {Operation} Credit operation.
	 * @private
	 */
	createCreditOperation(options) {
		const operation = this.createOperation(options.id, 'transfer');
		if (options.targetAddress)
			operation.account = new AccountIdentifier(options.targetAddress);
		else
			operation.account = this.publicKeyStringToAccountIdentifier(options.targetPublicKey);

		operation.amount = new Amount(options.amount.toString(), options.currency);
		return operation;
	}

	/**
	 * Creates a debit operation.
	 * @param {object} options Debit options.
	 * @returns {Operation} Debit operation.
	 * @private
	 */
	createDebitOperation(options) {
		const operation = this.createOperation(options.id, 'transfer');
		operation.account = this.publicKeyStringToAccountIdentifier(options.sourcePublicKey);
		operation.amount = new Amount((-options.amount).toString(), options.currency);
		return operation;
	}

	/**
	 * Looks up the default network currency.
	 * @returns {Currency} Fee currency.
	 * @private
	 */
	async lookupFeeCurrency() {
		const { currency } = await this.options.lookupCurrency('currencyMosaicId');
		return currency;
	}

	/**
	 * Parses a transaction into a rosetta transaction.
	 * @param {object} transaction Transaction to process (REST object model is assumed).
	 * @param {object} metadata Transaction metadata.
	 * @returns {Transaction} Rosetta transaction.
	 */
	async parseTransactionAsRosettaTransaction(transaction, metadata) {
		const { operations } = await this.parseTransaction(transaction, metadata);
		return new Transaction(new TransactionIdentifier(metadata.hash), operations);
	}

	/**
	 * Parses a transaction into operations.
	 * @param {object} transaction Transaction to process (REST object model is assumed).
	 * @returns {Array<Operation>} Transaction operations.
	 */
	async parseTransaction(transaction) {
		const allSignerPublicKeyStringSet = new Set();

		const operations = [];
		const appendOperation = operation => {
			// filter out zero transfers
			if ('transfer' === operation.type && '0' === operation.amount.value)
				return;

			operation.operation_identifier.index = operations.length;
			operations.push(operation);
		};

		const processSubTransaction = async subTransaction => {
			const subOperations = await this.parseTransactionInternal(subTransaction);
			subOperations.forEach(appendOperation);
		};

		if (transaction.otherTrans) {
			await processSubTransaction(transaction.otherTrans);

			allSignerPublicKeyStringSet.add(transaction.otherTrans.signer);
			let cosignerFeeSum = 0;
			transaction.signatures.forEach(signature => {
				allSignerPublicKeyStringSet.add(signature.signer);
				cosignerFeeSum += parseInt(signature.fee, 10);

				const cosignOperation = this.createOperation(operations.length, 'cosign');
				cosignOperation.account = this.publicKeyStringToAccountIdentifier(signature.signer);
				operations.push(cosignOperation);
			});

			if (this.options.includeFeeOperation) {
				const feeCurrency = await this.lookupFeeCurrency();
				appendOperation(this.createDebitOperation({
					sourcePublicKey: transaction.otherTrans.signer,
					amount: parseInt(transaction.fee, 10) + cosignerFeeSum,
					currency: feeCurrency
				}));
			}
		} else {
			await processSubTransaction(transaction);

			allSignerPublicKeyStringSet.add(transaction.signer);

			if (this.options.includeFeeOperation) {
				const feeCurrency = await this.lookupFeeCurrency();
				appendOperation(this.createDebitOperation({
					sourcePublicKey: transaction.signer,
					amount: transaction.fee,
					currency: feeCurrency
				}));
			}
		}

		return {
			operations,
			signerAddresses: [...allSignerPublicKeyStringSet].map(publicKeyString =>
				this.network.publicKeyToAddress(new PublicKey(publicKeyString)))
		};
	}

	/**
	 * Parses a transaction into operations.
	 * @param {object} transaction Transaction to process (REST object model is assumed).
	 * @returns {Array<Operation>} Transaction operations.
	 * @private
	 */
	async parseTransactionInternal(transaction) {
		const operations = [];

		const pushDebitCreditOperations = (sourcePublicKey, targetAddress, amount, currency) => {
			operations.push(this.createDebitOperation({ sourcePublicKey, amount, currency }));
			operations.push(this.createCreditOperation({ targetAddress, amount, currency }));
		};

		const pushRegistrationFeeOperation = async feeName => {
			const amount = transaction[`${feeName}Fee`];
			const currency = await this.lookupFeeCurrency();

			pushDebitCreditOperations(transaction.signer, hexStringToUtfString(transaction[`${feeName}FeeSink`]), amount, currency);
		};

		const transactionType = transaction.type;
		if (models.TransactionType.TRANSFER.value === transactionType) {
			const pushTransferOperations = (amount, currency) => {
				pushDebitCreditOperations(transaction.signer, transaction.recipient, amount, currency);
			};

			if (1 === transaction.version || 0 === transaction.mosaics.length) {
				const { amount } = transaction;
				const currency = await this.lookupFeeCurrency();

				pushTransferOperations(amount, currency);
			} else {
				await Promise.all(transaction.mosaics.map(async mosaic => {
					const amount = (transaction.amount / 1000000) * mosaic.quantity;
					const { currency, levy } = await this.options.lookupCurrency(mosaic.mosaicId);

					pushTransferOperations(amount, currency);

					if (levy) {
						const levyAmount = levy.isAbsolute
							? levy.fee
							: amount * levy.fee / 10000;
						pushDebitCreditOperations(transaction.signer, levy.recipientAddress, levyAmount, levy.currency);
					}
				}));
			}
		} else if (models.TransactionType.MULTISIG_ACCOUNT_MODIFICATION.value === transactionType) {
			const operation = this.createOperation(undefined, 'multisig');
			operation.account = this.publicKeyStringToAccountIdentifier(transaction.signer);
			const addressAdditions = [];
			const addressDeletions = [];

			transaction.modifications.forEach(modification => {
				const cosignatoryAddress = this.publicKeyStringToAccountIdentifier(modification.cosignatoryPublicKey).address;
				if (models.MultisigAccountModificationType.ADD_COSIGNATORY.value === modification.modificationType)
					addressAdditions.push(cosignatoryAddress);
				else
					addressDeletions.push(cosignatoryAddress);
			});

			operation.metadata = { addressAdditions, addressDeletions };
			if (transaction.minCosignatories)
				operation.metadata.minApprovalDelta = transaction.minCosignatories.relativeChange;

			operations.push(operation);
		} else if (models.TransactionType.MOSAIC_SUPPLY_CHANGE.value === transactionType) {
			const amount = transaction.delta;
			const { currency } = this.options.lookupCurrency(transaction.mosaicId);

			operations.push(this.createCreditOperation({
				targetPublicKey: transaction.signer,
				amount: models.MosaicSupplyChangeAction.INCREASE.value === transaction.action ? amount : -amount,
				currency
			}));
		} else if (models.TransactionType.NAMESPACE_REGISTRATION.value === transactionType) {
			await pushRegistrationFeeOperation('rental');
		} else if (models.TransactionType.MOSAIC_DEFINITION.value === transactionType) {
			await pushRegistrationFeeOperation('creation');

			const findProperty = (properties, name) => properties.find(property => name === property.name);

			const { mosaicDefinition } = transaction;
			const initialSupplyProperty = findProperty(mosaicDefinition.properties, 'initialSupply');
			const divisibilityProperty = findProperty(mosaicDefinition.properties, 'divisibility');
			if (initialSupplyProperty) {
				const namespaceName = mosaicDefinition.id.namespaceId;
				const mosaicName = mosaicDefinition.id.name;

				const currency = new Currency(
					`${namespaceName}.${mosaicName}`,
					undefined === divisibilityProperty ? 0 : parseInt(divisibilityProperty.value, 10)
				);

				operations.push(this.createCreditOperation({
					targetPublicKey: transaction.signer,
					amount: parseInt(initialSupplyProperty.value, 10),
					currency
				}));
			}
		}

		return operations;
	}
}
