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

import { createLookupCurrencyFunction, getBlockchainDescriptor } from './rosettaUtils.js';
import AccountIdentifier from '../openApi/model/AccountIdentifier.js';
import Amount from '../openApi/model/Amount.js';
import Operation from '../openApi/model/Operation.js';
import OperationIdentifier from '../openApi/model/OperationIdentifier.js';
import Transaction from '../openApi/model/Transaction.js';
import TransactionIdentifier from '../openApi/model/TransactionIdentifier.js';
import { NetworkLocator, PublicKey, utils } from 'symbol-sdk';
import { Address, Network, models } from 'symbol-sdk/symbol';

const idStringToBigInt = str => BigInt(`0x${str}`);
const encodeDecodedAddress = address => new Address(utils.hexToUint8(address)).toString();

// region convertTransactionSdkJsonToRestJson

/**
 * Converts a transaction SDK JSON model into a transaction REST JSON model.
 * @param {object} transactionJson Transaction SDK JSON model.
 * @returns {object} Transaction REST JSON model.
 */
export const convertTransactionSdkJsonToRestJson = transactionJson => {
	const decStringToHexString = value => BigInt(value).toString(16).padStart(16, '0').toUpperCase();

	if (transactionJson.mosaics) {
		transactionJson.mosaics = transactionJson.mosaics.map(mosaic => ({
			id: decStringToHexString(mosaic.mosaicId),
			amount: mosaic.amount
		}));
	}

	if (transactionJson.mosaic) {
		Object.assign(transactionJson, transactionJson.mosaic);
		delete transactionJson.mosaic;
	}

	if (transactionJson.mosaicId)
		transactionJson.mosaicId = decStringToHexString(transactionJson.mosaicId);

	if (transactionJson.transactions) {
		transactionJson.transactions = transactionJson.transactions.map(subTransaction => ({
			transaction: convertTransactionSdkJsonToRestJson(subTransaction)
		}));
	}

	if (transactionJson.fee) {
		transactionJson.maxFee = transactionJson.fee;
		delete transactionJson.fee;
	}

	return transactionJson;
};

// endregion

/**
 * Parses catapult models into rosetta operations.
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
			resolveAddress: (address, transactionLocation) => services.proxy.resolveAddress(address, transactionLocation),
			...options
		});
	}

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
			operation.account = new AccountIdentifier(encodeDecodedAddress(options.targetAddress));
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
		if (options.sourceAddress)
			operation.account = new AccountIdentifier(encodeDecodedAddress(options.sourceAddress));
		else
			operation.account = this.publicKeyStringToAccountIdentifier(options.sourcePublicKey);

		operation.amount = new Amount((-options.amount).toString(), options.currency);
		return operation;
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
	 * @param {object} metadata Transaction metadata.
	 * @returns {Array<Operation>} Transaction operations.
	 */
	async parseTransaction(transaction, metadata = {}) {
		const allSignerPublicKeyStrings = [];
		allSignerPublicKeyStrings.push(transaction.signerPublicKey);

		const makeTransactionLocation = (secondaryId = undefined) => {
			if (undefined === metadata.height || '0' === metadata.height)
				return undefined;

			return {
				height: BigInt(metadata.height),
				primaryId: metadata.index + 1,
				secondaryId: undefined === secondaryId ? 0 : secondaryId + 1
			};
		};

		const operations = [];
		const appendOperation = operation => {
			// filter out zero transfers
			if ('transfer' === operation.type && '0' === operation.amount.value)
				return;

			operation.operation_identifier.index = operations.length;
			operations.push(operation);
		};

		if (transaction.transactions) {
			const operationGroups = Array(transaction.transactions.length);
			await Promise.all(transaction.transactions.map(async (subTransaction, i) => {
				// due to async nature, operations from previous transaction might not be added yet
				// so simply using operations.length would yield wrong result
				// instead, renumber the operations below
				const subOperations = await this.parseTransactionInternal(
					subTransaction.transaction,
					subTransaction.meta && undefined !== subTransaction.meta.index
						? makeTransactionLocation(subTransaction.meta.index)
						: undefined
				);
				operationGroups[i] = subOperations;
			}));

			operationGroups.forEach(operationGroup => {
				operationGroup.forEach(appendOperation);
			});

			transaction.cosignatures.forEach(cosignature => {
				if (!allSignerPublicKeyStrings.includes(cosignature.signerPublicKey))
					allSignerPublicKeyStrings.push(cosignature.signerPublicKey);

				const cosignOperation = this.createOperation(operations.length, 'cosign');
				cosignOperation.account = this.publicKeyStringToAccountIdentifier(cosignature.signerPublicKey);
				operations.push(cosignOperation);
			});
		} else {
			const subOperations = await this.parseTransactionInternal(transaction, makeTransactionLocation());
			subOperations.forEach(appendOperation);
		}

		if (this.options.includeFeeOperation) {
			const amount = undefined !== metadata.feeMultiplier
				? BigInt(metadata.feeMultiplier) * BigInt(transaction.size)
				: BigInt(transaction.maxFee);
			const feeCurrency = await this.options.lookupCurrency('currencyMosaicId');

			appendOperation(this.createDebitOperation({
				sourcePublicKey: transaction.signerPublicKey,
				amount,
				currency: feeCurrency
			}));
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
	 * @param {object} transactionLocation Location of transaction for which to perform the resolution.
	 * @returns {Array<Operation>} Transaction operations.
	 * @private
	 */
	async parseTransactionInternal(transaction, transactionLocation = undefined) {
		const operations = [];

		const lookupCurrency = mosaicIdString => this.options.lookupCurrency(idStringToBigInt(mosaicIdString), transactionLocation);
		const resolveAddress = address => this.options.resolveAddress(address, transactionLocation);
		const resolveAllAddresses = addresses => Promise.all(addresses.map(address => resolveAddress(address).then(encodeDecodedAddress)));

		const transactionType = transaction.type;
		if (models.TransactionType.TRANSFER.value === transactionType) {
			await Promise.all(transaction.mosaics.map(async mosaic => {
				const amount = BigInt(mosaic.amount);
				const currency = await lookupCurrency(mosaic.id);
				const targetAddress = await resolveAddress(transaction.recipientAddress);

				operations.push(this.createDebitOperation({
					sourcePublicKey: transaction.signerPublicKey,
					amount,
					currency
				}));
				operations.push(this.createCreditOperation({
					targetAddress,
					amount,
					currency
				}));
			}));
		} else if (models.TransactionType.MULTISIG_ACCOUNT_MODIFICATION.value === transactionType) {
			const operation = this.createOperation(undefined, 'multisig');
			operation.account = this.publicKeyStringToAccountIdentifier(transaction.signerPublicKey);
			const addressAdditions = await resolveAllAddresses(transaction.addressAdditions);
			const addressDeletions = await resolveAllAddresses(transaction.addressDeletions);

			operation.metadata = {
				minApprovalDelta: transaction.minApprovalDelta,
				minRemovalDelta: transaction.minRemovalDelta,
				addressAdditions,
				addressDeletions
			};
			operations.push(operation);
		} else if (models.TransactionType.MOSAIC_SUPPLY_CHANGE.value === transactionType) {
			const amount = BigInt(transaction.delta);
			const currency = await lookupCurrency(transaction.mosaicId);

			operations.push(this.createCreditOperation({
				targetPublicKey: transaction.signerPublicKey,
				amount: models.MosaicSupplyChangeAction.INCREASE.value === transaction.action ? amount : -amount,
				currency
			}));
		} else if (models.TransactionType.MOSAIC_SUPPLY_REVOCATION.value === transactionType) {
			const amount = BigInt(transaction.amount);
			const currency = await lookupCurrency(transaction.mosaicId);
			const sourceAddress = await resolveAddress(transaction.sourceAddress);

			operations.push(this.createDebitOperation({
				sourceAddress,
				amount,
				currency
			}));
			operations.push(this.createCreditOperation({
				targetPublicKey: transaction.signerPublicKey,
				amount,
				currency
			}));
		}

		return operations;
	}

	/**
	 * Parses a receipt into operations.
	 * @param {object} receipt Receipt to process (REST object model is assumed).
	 * @returns {Array<Operation>} Receipt operations.
	 */
	async parseReceipt(receipt) {
		if (undefined === receipt.amount)
			return { operations: [] };

		const amount = BigInt(receipt.amount);
		if (0n === amount)
			return { operations: [] };

		const currency = await this.options.lookupCurrency(idStringToBigInt(receipt.mosaicId));

		const operations = [];
		const makeOptions = options => ({
			id: operations.length,
			amount,
			currency,
			...options
		});

		const basicReceiptType = receipt.type & 0xF000;
		if (0x1000 === basicReceiptType) { // transfer
			operations.push(this.createDebitOperation(makeOptions({ sourceAddress: receipt.senderAddress })));
			operations.push(this.createCreditOperation(makeOptions({ targetAddress: receipt.recipientAddress })));
		} else if (0x2000 === basicReceiptType) { // credit
			operations.push(this.createCreditOperation(makeOptions({ targetAddress: receipt.targetAddress })));
		} else if (0x3000 === basicReceiptType) { // debit
			operations.push(this.createDebitOperation(makeOptions({ sourceAddress: receipt.targetAddress })));
		}

		return { operations };
	}
}
