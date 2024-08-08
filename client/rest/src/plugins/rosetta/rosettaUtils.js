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

import Amount from './openApi/model/Amount.js';
import RosettaApiError from './openApi/model/Error.js';
import Operation from './openApi/model/Operation.js';
import RosettaPublicKey from './openApi/model/PublicKey.js';
import { sendJson } from '../../routes/simpleSend.js';

// region RosettaError / RosettaErrorFactory

/**
 * Error thrown when a rosetta endpoint encounters an error.
 */
class RosettaError extends Error {
	constructor(code, message, retriable) {
		super(message);

		this.apiError = new RosettaApiError(code, message, retriable);
	}
}

/**
 * Factory for creating rosetta errors.
 */
export class RosettaErrorFactory {
	static get UNSUPPORTED_NETWORK() {
		return new RosettaError(1, 'unsupported network', false);
	}

	static get UNSUPPORTED_CURVE() {
		return new RosettaError(2, 'unsupported curve', false);
	}

	static get INVALID_PUBLIC_KEY() {
		return new RosettaError(3, 'invalid public key', false);
	}

	static get INVALID_REQUEST_DATA() {
		return new RosettaError(4, 'invalid request data', false);
	}

	static get CONNECTION_ERROR() {
		return new RosettaError(5, 'unable to connect to network', true);
	}

	static get SYNC_DURING_OPERATION() {
		return new RosettaError(6, 'sync was detected during operation', true);
	}

	static get NOT_SUPPORTED_ERROR() {
		return new RosettaError(99, 'operation is not supported in rosetta', false);
	}

	static get INTERNAL_SERVER_ERROR() {
		return new RosettaError(100, 'internal server error', false);
	}
}

// endregion

// region RosettaPublicKeyProcessor

/**
 * Rosetta public key processor.
 */
export class RosettaPublicKeyProcessor {
	/**
	 * Creates a public key processor.
	 * @param {string} requiredCurveType Required public key curve type.
	 * @param {object} network Network using public keys.
	 * @param {Function} PublicKeyClass Public key class.
	 */
	constructor(requiredCurveType, network, PublicKeyClass) {
		this.requiredCurveType = requiredCurveType;
		this.network = network;
		this.PublicKeyClass = PublicKeyClass;
	}

	/**
	 * Parses a rosetta public key.
	 * @param {RosettaPublicKey} rosettaPublicKey Rosetta public key.
	 * @returns {this.PublicKeyClass} Parsed public key.
	 */
	parsePublicKey(rosettaPublicKey) {
		if (this.requiredCurveType !== rosettaPublicKey.curve_type)
			throw RosettaErrorFactory.UNSUPPORTED_CURVE;

		try {
			return new this.PublicKeyClass(rosettaPublicKey.hex_bytes);
		} catch (err) {
			throw RosettaErrorFactory.INVALID_PUBLIC_KEY;
		}
	}

	/**
	 * Processes rosetta public keys into an address to public key map.
	 * @param {Array<RosettaPublicKey>} rosettaPublicKeys Rosetta public keys.
	 * @returns {Map<string, this.PublicKeyClass>} Address to public key map.
	 */
	buildAddressToPublicKeyMap(rosettaPublicKeys) {
		const addressToPublicKeyMap = {};
		for (let i = 0; i < rosettaPublicKeys.length; ++i) {
			const publicKey = this.parsePublicKey(rosettaPublicKeys[i]);
			const address = this.network.publicKeyToAddress(publicKey);
			addressToPublicKeyMap[address.toString()] = publicKey;
		}

		return addressToPublicKeyMap;
	}
}

// endregion

// region rosettaPostRouteWithNetwork

/**
 * Orchestrates a rosetta POST route by validating request data, including network, before calling user handler.
 * @param {object} blockchainDescriptor Blockchain descriptor.
 * @param {object} Request Type of request object.
 * @param {Function} handler User callback that is called with request data after validating request.
 * @returns {Function} Restify POST handler.
 */
export const rosettaPostRouteWithNetwork = (blockchainDescriptor, Request, handler) => async (req, res, next) => {
	const send = data => {
		const sender = sendJson(res, next);
		return data instanceof RosettaError ? sender(data.apiError, 500) : sender(data, undefined);
	};

	try {
		Request.validateJSON(req.body);
	} catch (err) {
		return send(RosettaErrorFactory.INVALID_REQUEST_DATA);
	}

	const typedRequest = Request.constructFromObject(req.body);
	const isTargetNetwork = networkIdentifier =>
		blockchainDescriptor.blockchain === networkIdentifier.blockchain && blockchainDescriptor.network === networkIdentifier.network;
	if (!isTargetNetwork(typedRequest.network_identifier))
		return send(RosettaErrorFactory.UNSUPPORTED_NETWORK);

	try {
		const response = await handler(typedRequest);
		return send(response);
	} catch (err) {
		return send(err instanceof RosettaError ? err : RosettaErrorFactory.INTERNAL_SERVER_ERROR);
	}
};

// endregion

// region extractTransferDescriptorAt

/**
 * Extracts a transfer descriptor from a set of rosetta operations at specified index.
 * @param {Array<Operation>} operations Rosetta operations.
 * @param {number} index Transfer start index.
 * @returns {object} Transfer descriptor.
 */
export const extractTransferDescriptorAt = (operations, index) => {
	if (index + 1 >= operations.length)
		throw RosettaErrorFactory.INVALID_REQUEST_DATA;

	const operation1 = operations[index];
	const operation2 = operations[index + 1];
	if ('transfer' !== operation1.type || 'transfer' !== operation2.type)
		throw RosettaErrorFactory.INVALID_REQUEST_DATA;

	const amount = BigInt(operation1.amount.value);
	if (0n !== amount + BigInt(operation2.amount.value))
		throw RosettaErrorFactory.INVALID_REQUEST_DATA;

	if (0n < amount) {
		return {
			senderAddress: operation2.account.address,
			recipientAddress: operation1.account.address,
			amount
		};
	}

	return {
		senderAddress: operation1.account.address,
		recipientAddress: operation2.account.address,
		amount: -amount
	};
};

// endregion

// evaluateOperationsAndUpdateAmounts

/**
 * Updates an array of amounts for an account by applying an array of operations.
 * @param {string} address Account address.
 * @param {Array<Amount>} amounts Initial amounts to update.
 * @param {Array<Operation>} operations Rosetta operations.
 */
export const evaluateOperationsAndUpdateAmounts = (address, amounts, operations) => {
	const sumAmountsValue = (lhs, rhs) => (BigInt(lhs.value) + BigInt(rhs.value)).toString();

	const userOperations = operations.filter(operation => address === operation.account.address && undefined !== operation.amount);

	// combine the operations by each currency
	const currencyAmountMap = new Map();
	userOperations.forEach(operation => {
		const currentSumAmount = currencyAmountMap.get(operation.amount.currency.symbol) || new Amount('0', operation.amount.currency);
		currentSumAmount.value = sumAmountsValue(currentSumAmount, operation.amount);
		currencyAmountMap.set(operation.amount.currency.symbol, currentSumAmount);
	});

	// update the current mosaics
	amounts.forEach(amount => {
		if (currencyAmountMap.has(amount.currency.symbol)) {
			amount.value = sumAmountsValue(amount, currencyAmountMap.get(amount.currency.symbol));
			currencyAmountMap.delete(amount.currency.symbol);
		}
	});

	amounts.push(...currencyAmountMap.values());
};

// endregion
