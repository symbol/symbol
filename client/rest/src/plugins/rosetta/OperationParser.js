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

import AccountIdentifier from './openApi/model/AccountIdentifier.js';
import Amount from './openApi/model/Amount.js';
import Operation from './openApi/model/Operation.js';
import OperationIdentifier from './openApi/model/OperationIdentifier.js';
import { PublicKey, utils } from 'symbol-sdk';
import { Address, Network, models } from 'symbol-sdk/symbol';

const encodeDecodedAddress = address => new Address(utils.hexToUint8(address)).toString();

const createOperation = (id, type) => {
	const operation = new Operation(new OperationIdentifier(id), type);
	operation.status = 'success';
	return operation;
};

/**
 * Parses a catapult models into rosetta operations.
 */
export default class OperationParser {
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
		const operation = createOperation(options.id, 'transfer');
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
		const operation = createOperation(options.id, 'transfer');
		if (options.sourceAddress)
			operation.account = new AccountIdentifier(encodeDecodedAddress(options.sourceAddress));
		else
			operation.account = this.publicKeyStringToAccountIdentifier(options.sourcePublicKey);

		operation.amount = new Amount((-options.amount).toString(), options.currency);
		return operation;
	}

	/**
	 * Parses a transaction into operations.
	 * @param {object} transaction Transaction to process (REST object model is assumed).
	 * @param {object} metadata Transaction metadata.
	 * @returns {Array<Operation>} Transaction operations.
	 */
	async parseTransaction(transaction, metadata = {}) {
		const allSignerPublicKeyStringSet = new Set();
		allSignerPublicKeyStringSet.add(transaction.signerPublicKey);

		const operations = [];
		if (transaction.transactions) {
			const operationGroups = Array(transaction.transactions.length);
			await Promise.all(transaction.transactions.map(async (subTransaction, i) => {
				allSignerPublicKeyStringSet.add(subTransaction.signerPublicKey);

				// due to async nature, operations from previous transaction might not be added yet
				// so simply using operations.length would yield wrong result
				// instead, renumber the operations below
				const subOperations = await this.parseTransactionInternal(0, subTransaction);
				operationGroups[i] = subOperations;
			}));

			operationGroups.forEach(operationGroup => {
				operationGroup.forEach(operation => {
					operation.operation_identifier.index = operations.length;
					operations.push(operation);
				});
			});

			transaction.cosignatures.forEach(cosignature => {
				if (allSignerPublicKeyStringSet.has(cosignature.signerPublicKey))
					return;

				allSignerPublicKeyStringSet.add(cosignature.signerPublicKey);

				const cosignOperation = createOperation(operations.length, 'cosign');
				cosignOperation.account = this.publicKeyStringToAccountIdentifier(cosignature.signerPublicKey);
				operations.push(cosignOperation);
			});
		} else {
			const subOperations = await this.parseTransactionInternal(operations.length, transaction);
			operations.push(...subOperations);
		}

		if (this.options.includeFeeOperation) {
			const amount = metadata.feeMultiplier ? BigInt(metadata.feeMultiplier) * BigInt(transaction.size) : BigInt(transaction.maxFee);
			const feeCurrency = await this.options.lookupCurrency('currencyMosaicId');

			operations.push(this.createDebitOperation({
				id: operations.length,
				sourcePublicKey: transaction.signerPublicKey,
				amount,
				currency: feeCurrency
			}));
		}

		return {
			operations,
			signerAddresses: [...allSignerPublicKeyStringSet].map(publicKeyString =>
				this.network.publicKeyToAddress(new PublicKey(publicKeyString)))
		};
	}

	/**
	 * Parses a transaction into operations.
	 * @param {number} startOperationId First operation id to use.
	 * @param {object} transaction Transaction to process (REST object model is assumed).
	 * @returns {Array<Operation>} Transaction operations.
	 * @private
	 */
	async parseTransactionInternal(startOperationId, transaction) {
		let id = startOperationId;
		const operations = [];

		const transactionType = transaction.type;
		if (models.TransactionType.TRANSFER.value === transactionType) {
			await Promise.all(transaction.mosaics.map(async mosaic => {
				const amount = BigInt(mosaic.amount);
				const currency = await this.options.lookupCurrency(BigInt(mosaic.mosaicId));

				operations.push(this.createDebitOperation({
					id: id++,
					sourcePublicKey: transaction.signerPublicKey,
					amount,
					currency
				}));
				operations.push(this.createCreditOperation({
					id: id++,
					targetAddress: transaction.recipientAddress,
					amount,
					currency
				}));
			}));
		} else if (models.TransactionType.MULTISIG_ACCOUNT_MODIFICATION.value === transactionType) {
			const operation = createOperation(id++, 'multisig');
			operation.account = this.publicKeyStringToAccountIdentifier(transaction.signerPublicKey);
			operation.metadata = {
				minApprovalDelta: transaction.minApprovalDelta,
				minRemovalDelta: transaction.minRemovalDelta,
				addressAdditions: transaction.addressAdditions.map(encodeDecodedAddress),
				addressDeletions: transaction.addressDeletions.map(encodeDecodedAddress)
			};
			operations.push(operation);
		} else if (models.TransactionType.MOSAIC_SUPPLY_REVOCATION.value === transactionType) {
			const amount = BigInt(transaction.amount);
			const currency = await this.options.lookupCurrency(BigInt(transaction.mosaicId));

			operations.push(this.createDebitOperation({
				id: id++,
				sourceAddress: transaction.sourceAddress,
				amount,
				currency
			}));
			operations.push(this.createCreditOperation({
				id: id++,
				targetPublicKey: transaction.signerPublicKey,
				amount,
				currency
			}));
		} else if (models.TransactionType.MOSAIC_SUPPLY_CHANGE.value === transactionType) {
			const amount = BigInt(transaction.delta);
			const currency = await this.options.lookupCurrency(BigInt(transaction.mosaicId));

			operations.push(this.createCreditOperation({
				id: id++,
				targetPublicKey: transaction.signerPublicKey,
				amount: models.MosaicSupplyChangeAction.INCREASE.value === transaction.action ? amount : -amount,
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
		const operations = [];

		const amount = BigInt(receipt.amount);
		const currency = await this.options.lookupCurrency(BigInt(receipt.mosaicId));

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
