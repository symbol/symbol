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

import Currency from '../openApi/model/Currency.js';

/**
 * Extracts blockchain descriptor from services configuration.
 * @param {object} config Services configuration.
 * @returns {object} Blockchain descriptor.
 */
export const getBlockchainDescriptor = config => ({
	blockchain: 'Symbol',
	network: config.network.name
});

/**
 * Creates the lookup currency function used by the operation parser.
 * @param {object} proxy Catapult proxy.
 * @returns {Function} Currency lookup function.
 */
export const createLookupCurrencyFunction = proxy => async (unresolvedMosaicId, transactionLocation) => {
	const resolvedMosaicId = 'currencyMosaicId' === unresolvedMosaicId
		? BigInt((await proxy.networkProperties()).chain.currencyMosaicId)
		: await proxy.resolveMosaicId(unresolvedMosaicId, transactionLocation);
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
