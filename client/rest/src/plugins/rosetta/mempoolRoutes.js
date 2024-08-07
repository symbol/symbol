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
import MempoolResponse from './openApi/model/MempoolResponse.js';
import MempoolTransactionRequest from './openApi/model/MempoolTransactionRequest.js';
import MempoolTransactionResponse from './openApi/model/MempoolTransactionResponse.js';
import NetworkRequest from './openApi/model/NetworkRequest.js';
import TransactionIdentifier from './openApi/model/TransactionIdentifier.js';
import {
	createLookupCurrencyFunction, rosettaPostRouteWithNetwork
} from './rosettaUtils.js';
import { NetworkLocator } from 'symbol-sdk';
import { Network } from 'symbol-sdk/symbol';

export default {
	register: (server, db, services) => {
		const PAGE_SIZE = 100;

		const networkName = services.config.network.name;
		const network = NetworkLocator.findByName(Network.NETWORKS, networkName);
		const lookupCurrency = createLookupCurrencyFunction(services.proxy);
		const parser = new OperationParser(network, {
			includeFeeOperation: true,
			operationStatus: 'success',
			lookupCurrency,
			resolveAddress: (address, transactionLocation) => services.proxy.resolveAddress(address, transactionLocation)
		});

		server.post('/mempool', rosettaPostRouteWithNetwork(networkName, NetworkRequest, async () => {
			const transactions = await services.proxy.fetchAll('transactions/unconfirmed', PAGE_SIZE);

			const response = new MempoolResponse();
			response.transaction_identifiers = transactions.map(transaction => new TransactionIdentifier(transaction.meta.hash));
			return response;
		}));

		server.post('/mempool/transaction', rosettaPostRouteWithNetwork(networkName, MempoolTransactionRequest, async typedRequest => {
			const transactionHash = typedRequest.transaction_identifier.hash;
			const transaction = await services.proxy.fetch(`transactions/unconfirmed/${transactionHash}`);
			const response = new MempoolTransactionResponse();
			response.transaction = await parser.parseTransactionAsRosettaTransaction(transaction.transaction, transaction.meta);
			return response;
		}));
	}
};
