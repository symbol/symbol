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

import { createLookupCurrencyFunction, getBlockchainDescriptor, mosaicIdToString } from './rosettaUtils.js';
import AccountIdentifier from '../openApi/model/AccountIdentifier.js';
import Amount from '../openApi/model/Amount.js';
import Currency from '../openApi/model/Currency.js';
import Operation from '../openApi/model/Operation.js';
import OperationIdentifier from '../openApi/model/OperationIdentifier.js';
import Transaction from '../openApi/model/Transaction.js';
import TransactionIdentifier from '../openApi/model/TransactionIdentifier.js';
import { NetworkLocator, PublicKey, utils } from 'symbol-sdk';
import { Network, models } from 'symbol-sdk/nem';

// region convertTransactionSdkJsonToRestJson

/**
 * Converts a transaction SDK JSON model into a transaction REST JSON model.
 * @param {object} transactionJson Transaction SDK JSON model.
 * @returns {object} Transaction REST JSON model.
 */
export const convertTransactionSdkJsonToRestJson = transactionJson => {
	const hexStringToUtfString = hex => new TextDecoder().decode(utils.hexToUint8(hex));

	const convertMosaicIdSdkJsonToRestJson = mosaicId => ({
		namespaceId: hexStringToUtfString(mosaicId.namespaceId.name),
		name: hexStringToUtfString(mosaicId.name)
	});

	const renameProperty = (originalName, newName, transform = undefined) => {
		const value = transactionJson[originalName];
		transactionJson[newName] = transform ? transform(value) : value;
		delete transactionJson[originalName];
	};

	if (transactionJson.network) {
		transactionJson.version = utils.bytesToInt(new Uint8Array([transactionJson.version, 0, 0, transactionJson.network]), 4);
		delete transactionJson.network;
	}

	if (transactionJson.mosaics) {
		transactionJson.mosaics = transactionJson.mosaics.map(mosaic => mosaic.mosaic).map(mosaic => ({
			quantity: mosaic.amount,
			mosaicId: convertMosaicIdSdkJsonToRestJson(mosaic.mosaicId)
		}));
	}

	if (transactionJson.mosaicId)
		transactionJson.mosaicId = convertMosaicIdSdkJsonToRestJson(transactionJson.mosaicId);

	if (transactionJson.modifications) {
		transactionJson.modifications = transactionJson.modifications.map(modification => modification.modification).map(modification => ({
			modificationType: modification.modificationType,
			cosignatoryAccount: modification.cosignatoryPublicKey
		}));
	}

	if (transactionJson.signerPublicKey)
		renameProperty('signerPublicKey', 'signer');

	if (transactionJson.innerTransaction)
		renameProperty('innerTransaction', 'otherTrans', convertTransactionSdkJsonToRestJson);

	if (transactionJson.cosignatures) {
		renameProperty(
			'cosignatures',
			'signatures',
			cosignatures => cosignatures.map(cosignature => convertTransactionSdkJsonToRestJson(cosignature.cosignature))
		);
	}

	if (transactionJson.recipientAddress)
		renameProperty('recipientAddress', 'recipient', hexStringToUtfString);

	if (transactionJson.minApprovalDelta)
		renameProperty('minApprovalDelta', 'minCosignatories', minApprovalDelta => ({ relativeChange: minApprovalDelta }));

	if (transactionJson.rentalFeeSink)
		transactionJson.rentalFeeSink = hexStringToUtfString(transactionJson.rentalFeeSink);

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

	if (transactionJson.action)
		renameProperty('action', 'supplyType');

	return transactionJson;
};

// endregion

/**
 * Parses NEM models into rosetta operations.
 */
export class OperationParser {
	/**
	 * Creates a fully configured operation parser given REST services.
	 * @param {object} services REST services.
	 * @param {object} options Parser options.
	 * @returns {OperationParser} Operation parser.
	 */
	static createFromServices(services, options = {}) {
		const blockchainDescriptor = getBlockchainDescriptor(services.config);
		const network = NetworkLocator.findByName(Network.NETWORKS, blockchainDescriptor.network);
		const lookupCurrency = createLookupCurrencyFunction(services.proxy);
		return new OperationParser(network, {
			includeFeeOperation: true,
			lookupCurrency,
			...options
		});
	}

	/**
	 * Creates a parser.
	 * @param {Network} network NEM network.
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
		return new Transaction(new TransactionIdentifier(metadata.hash.data.toUpperCase()), operations);
	}

	/**
	 * Parses a transaction into operations.
	 * @param {object} transaction Transaction to process (REST object model is assumed).
	 * @param {object} metadata Transaction metadata.
	 * @returns {Array<Operation>} Transaction operations.
	 */
	async parseTransaction(transaction, metadata) {
		const allSignerPublicKeyStrings = [];

		const operations = [];
		const appendOperation = operation => {
			// filter out zero transfers
			if ('transfer' === operation.type && '0' === operation.amount.value)
				return;

			operation.operation_identifier.index = operations.length;
			operations.push(operation);
		};

		const processSubTransaction = async subTransaction => {
			const subOperations = await this.parseTransactionInternal(subTransaction, metadata);
			subOperations.forEach(appendOperation);
		};

		const pushCosignOperation = cosignerPublicKey => {
			const cosignOperation = this.createOperation(operations.length, 'cosign');
			cosignOperation.account = this.publicKeyStringToAccountIdentifier(cosignerPublicKey);
			operations.push(cosignOperation);
		};

		allSignerPublicKeyStrings.push(transaction.signer);

		if (transaction.otherTrans) {
			await processSubTransaction(transaction.otherTrans);

			pushCosignOperation(transaction.signer);

			let cosignerFeeSum = 0;
			transaction.signatures.forEach(signature => {
				allSignerPublicKeyStrings.push(signature.signer);
				pushCosignOperation(signature.signer);

				cosignerFeeSum += parseInt(signature.fee, 10);
			});

			if (this.options.includeFeeOperation) {
				const feeCurrency = await this.lookupFeeCurrency();
				appendOperation(this.createDebitOperation({
					sourcePublicKey: transaction.otherTrans.signer,
					amount: parseInt(transaction.fee, 10) + parseInt(transaction.otherTrans.fee, 10) + cosignerFeeSum,
					currency: feeCurrency
				}));
			}
		} else {
			await processSubTransaction(transaction);

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
			signerAddresses: allSignerPublicKeyStrings.map(publicKeyString =>
				this.network.publicKeyToAddress(new PublicKey(publicKeyString)))
		};
	}

	/**
	 * Parses a transaction into operations.
	 * @param {object} transaction Transaction to process (REST object model is assumed).
	 * @returns {Array<Operation>} Transaction operations.
	 * @param {object} transactionLocation Location of transaction for which to perform the resolution.
	 * @private
	 */
	async parseTransactionInternal(transaction, transactionLocation) {
		const operations = [];
		const lookupCurrency = mosaicId => this.options.lookupCurrency(mosaicId, transactionLocation);

		const pushDebitCreditOperations = (sourcePublicKey, targetAddress, amount, currency) => {
			operations.push(this.createDebitOperation({ sourcePublicKey, amount, currency }));
			operations.push(this.createCreditOperation({ targetAddress, amount, currency }));
		};

		const pushRegistrationFeeOperation = async feeName => {
			const amount = transaction[`${feeName}Fee`];
			const currency = await this.lookupFeeCurrency();

			pushDebitCreditOperations(transaction.signer, transaction[`${feeName}FeeSink`], amount, currency);
		};

		const transactionType = transaction.type;
		if (models.TransactionType.TRANSFER.value === transactionType) {
			const pushTransferOperations = (amount, currency) => {
				pushDebitCreditOperations(transaction.signer, transaction.recipient, amount, currency);
			};

			if (1 === (transaction.version & 0xFF) || 0 === transaction.mosaics.length) {
				const { amount } = transaction;
				const currency = await this.lookupFeeCurrency();

				pushTransferOperations(amount, currency);
			} else {
				await Promise.all(transaction.mosaics.map(async mosaic => {
					const amount = Math.trunc((transaction.amount * mosaic.quantity) / 1000000);
					const { currency, levy } = await lookupCurrency(mosaic.mosaicId);

					pushTransferOperations(amount, currency);

					if (levy) {
						const levyAmount = levy.isAbsolute
							? levy.fee
							: Math.trunc(amount * levy.fee / 10000);
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
				const cosignatoryAddress = this.publicKeyStringToAccountIdentifier(modification.cosignatoryAccount).address;
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
			const { currency } = await lookupCurrency(transaction.mosaicId);
			const amount = transaction.delta * (10 ** currency.decimals);

			operations.push(this.createCreditOperation({
				targetPublicKey: transaction.signer,
				amount: models.MosaicSupplyChangeAction.INCREASE.value === transaction.supplyType ? amount : -amount,
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

			const initialSupply = undefined === initialSupplyProperty ? 1000 : parseInt(initialSupplyProperty.value, 10);
			const currency = new Currency(
				mosaicIdToString(mosaicDefinition.id),
				undefined === divisibilityProperty ? 0 : parseInt(divisibilityProperty.value, 10)
			);

			operations.push(this.createCreditOperation({
				targetPublicKey: transaction.signer,
				amount: initialSupply * (10 ** currency.decimals),
				currency
			}));
		}

		return operations;
	}

	/**
	 * Parses a block into operations.
	 * @param {object} block Block to process (REST object model is assumed).
	 * @returns {Array<Operation>} Receipt operations.
	 */
	async parseBlock(block) {
		if (0 === block.totalFee)
			return { operations: [] };

		const currency = await this.lookupFeeCurrency();
		return {
			operations: [
				this.createCreditOperation({
					id: 0,
					amount: block.totalFee,
					currency,
					targetAddress: block.beneficiary
				})
			]
		};
	}
}
