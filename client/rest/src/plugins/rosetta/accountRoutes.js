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

import { OperationParser } from './OperationParser.js';
import AccountBalanceRequest from './openApi/model/AccountBalanceRequest.js';
import AccountBalanceResponse from './openApi/model/AccountBalanceResponse.js';
import AccountCoinsRequest from './openApi/model/AccountCoinsRequest.js';
import AccountCoinsResponse from './openApi/model/AccountCoinsResponse.js';
import Amount from './openApi/model/Amount.js';
import BlockIdentifier from './openApi/model/BlockIdentifier.js';
import Coin from './openApi/model/Coin.js';
import CoinIdentifier from './openApi/model/CoinIdentifier.js';
import {
	RosettaErrorFactory, createLookupCurrencyFunction, rosettaPostRouteWithNetwork, stitchBlockTransactions
} from './rosettaUtils.js';
import { NetworkLocator } from 'symbol-sdk';
import { Network } from 'symbol-sdk/symbol';

export default {
	register: (server, db, services) => {
		const PAGE_SIZE = 100;

		const networkName = services.config.network.name;
		const network = NetworkLocator.findByName(Network.NETWORKS, networkName);
		const lookupCurrency = createLookupCurrencyFunction(services.proxy);
		const getChainHeight = () => services.proxy.fetch('chain/info', json => json.height);
		const getBlockIdentifier = height => services.proxy.fetch(`blocks/${height}`)
			.then(blockInfo => new BlockIdentifier(Number(blockInfo.block.height), blockInfo.meta.hash));
		const parser = new OperationParser(network, {
			includeFeeOperation: true,
			lookupCurrency,
			resolveAddress: (unresolvedAddress, transactionLocation) =>
				services.proxy.resolveAddress(unresolvedAddress, transactionLocation)
		});

		const isCurrencyEqual = (lhs, rhs) => lhs.symbol === rhs.symbol && lhs.decimals === rhs.decimals;

		const mapMosaicsToRosettaAmounts = mosaics => Promise.all(mosaics.map(async mosaic => {
			const currency = await lookupCurrency(BigInt(`0x${mosaic.id}`));
			return new Amount(mosaic.amount, currency);
		}));

		const getAccountOperations = async (unconfirmedTransactions, address) => {
			const mempoolTransactions = await Promise.all(stitchBlockTransactions(unconfirmedTransactions).map(transaction =>
				parser.parseTransactionAsRosettaTransaction(transaction.transaction, transaction.meta)));
			const accountOperations = [];
			mempoolTransactions.forEach(transaction => {
				const balanceChangeOperations = transaction.operations.filter(operation =>
					operation.account.address === address && undefined !== operation.amount);
				accountOperations.push(...balanceChangeOperations);
			});

			return accountOperations;
		};

		const getAccountAmounts = async typedRequest => {
			if (typedRequest.block_identifier)
				throw RosettaErrorFactory.NOT_SUPPORTED_ERROR;

			const { address } = typedRequest.account_identifier;
			if (!network.isValidAddressString(address))
				throw RosettaErrorFactory.INVALID_REQUEST_DATA;

			const startChainHeight = await getChainHeight();

			const promises = [services.proxy.fetch(`accounts/${address}`, json => json.account.mosaics)];
			if (typedRequest.include_mempool)
				promises.push(services.proxy.fetchAll('transactions/unconfirmed?embedded=true', PAGE_SIZE));

			const [mosaics, unconfirmedTransactions] = await Promise.all(promises);

			const endChainHeight = await getChainHeight();

			if (startChainHeight !== endChainHeight)
				throw RosettaErrorFactory.SYNC_DURING_OPERATION;

			let amounts = await mapMosaicsToRosettaAmounts(mosaics);
			if (typedRequest.include_mempool && unconfirmedTransactions?.length) {
				const combineAmountsValue = (lhs, rhs) => (BigInt(lhs) + BigInt(rhs)).toString();

				// combine the operations by each currency
				const currencyAmountMap = new Map();
				const userOperations = await getAccountOperations(unconfirmedTransactions, address);
				userOperations.forEach(userOperation => {
					const storedValue = currencyAmountMap.get(userOperation.amount.currency.symbol)?.value || 0;
					userOperation.amount.value = combineAmountsValue(userOperation.amount.value, storedValue);
					currencyAmountMap.set(userOperation.amount.currency.symbol, userOperation.amount);
				});

				// update the current mosaics
				amounts.forEach(amount => {
					if (currencyAmountMap.has(amount.currency.symbol)) {
						amount.value = combineAmountsValue(amount.value, currencyAmountMap.get(amount.currency.symbol).value);
						currencyAmountMap.delete(amount.currency.symbol);
					}
				});

				amounts.push(...currencyAmountMap.values());
			}

			if (typedRequest.currencies) {
				amounts = amounts.filter(amount =>
					typedRequest.currencies.some(currency => isCurrencyEqual(amount.currency, currency)));
			}

			const blockIdentifier = await getBlockIdentifier(startChainHeight);
			return { blockIdentifier, amounts };
		};

		server.post('/account/balance', rosettaPostRouteWithNetwork(networkName, AccountBalanceRequest, async typedRequest => {
			const { blockIdentifier, amounts } = await getAccountAmounts(typedRequest);

			return new AccountBalanceResponse(blockIdentifier, amounts);
		}));

		server.post('/account/coins', rosettaPostRouteWithNetwork(networkName, AccountCoinsRequest, async typedRequest => {
			const { blockIdentifier, amounts } = await getAccountAmounts(typedRequest);

			const mapRosettaAmountToCoin = amount => new Coin(new CoinIdentifier(amount.currency.metadata.id), amount);
			const coins = await Promise.all(amounts.map(mapRosettaAmountToCoin));
			return new AccountCoinsResponse(blockIdentifier, coins);
		}));
	}
};
