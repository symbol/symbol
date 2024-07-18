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

import Currency from './openApi/model/Currency.js';
import RosettaApiError from './openApi/model/Error.js';
import { sendJson } from '../../routes/simpleSend.js';

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

	static get NOT_SUPPORTED_ERROR() {
		return new RosettaError(99, 'operation is not supported in rosetta', false);
	}

	static get INTERNAL_SERVER_ERROR() {
		return new RosettaError(100, 'internal server error', false);
	}
}

/**
 * Orchestrates a rosetta POST route by validating request data, including network, before calling user handler.
 * @param {string} networkName Network name.
 * @param {object} Request Type of request object.
 * @param {Function} handler User callback that is called with request data after validating request.
 * @returns {Function} Restify POST handler.
 */
export const rosettaPostRouteWithNetwork = (networkName, Request, handler) => async (req, res, next) => {
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
	const checkNetwork = networkIdentifier => ('Symbol' === networkIdentifier.blockchain && networkName === networkIdentifier.network);
	if (!checkNetwork(typedRequest.network_identifier))
		return send(RosettaErrorFactory.UNSUPPORTED_NETWORK);

	try {
		const response = await handler(typedRequest);
		return send(response);
	} catch (err) {
		return send(err instanceof RosettaError ? err : RosettaErrorFactory.INTERNAL_SERVER_ERROR);
	}
};

/**
 * Creates the lookup currency function used by the operation parser.
 * @param {object} proxy Catapult proxy.
 * @returns {Function} Currency lookup function.
 */
export const createLookupCurrencyFunction = proxy => async unresolvedMosaicId => {
	const resolvedMosaicId = 'currencyMosaicId' === unresolvedMosaicId
		? BigInt((await proxy.networkProperties()).chain.currencyMosaicId)
		: await proxy.resolveMosaicId(unresolvedMosaicId);
	const mosaicProperties = await proxy.mosaicProperties(resolvedMosaicId);

	const currency = new Currency(mosaicProperties.name, mosaicProperties.divisibility);
	currency.metadata = { id: mosaicProperties.id };
	return currency;
};

/**
 * Accepts an array of REST JSON transactions composed of both top level and embedded transactions.
 * Reduces the array by stitching embedded transactions into their containing aggregates.
 * @param {Array<object>} transactions REST JSON transactions composed of both top level and embedded transactions.
 * @returns {Array<object>} REST JSON transactions composed of only top level transactions.
 */
export const stitchBlockTransactions = transactions => {
	// first pass - group embedded transactions
	const aggregateHashToEmbeddedTransactionsMap = new Map();
	transactions.forEach(transaction => {
		if (!transaction.meta.aggregateHash)
			return;

		if (!aggregateHashToEmbeddedTransactionsMap.has(transaction.meta.aggregateHash))
			aggregateHashToEmbeddedTransactionsMap.set(transaction.meta.aggregateHash, []);

		aggregateHashToEmbeddedTransactionsMap.get(transaction.meta.aggregateHash).push(transaction);
	});

	// second pass - filter out embedded transactions and attach embedded transactions to aggregates
	return transactions.filter(transaction => {
		if (aggregateHashToEmbeddedTransactionsMap.has(transaction.meta.hash)) {
			const embeddedTransactions = aggregateHashToEmbeddedTransactionsMap.get(transaction.meta.hash);
			embeddedTransactions.sort((lhs, rhs) => lhs.meta.index - rhs.meta.index);
			transaction.transaction.transactions = embeddedTransactions;
		}

		return transaction.meta.hash;
	});
};
