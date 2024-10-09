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
import { getBlockchainDescriptor } from './rosettaUtils.js';
import MempoolResponse from '../openApi/model/MempoolResponse.js';
import MempoolTransactionRequest from '../openApi/model/MempoolTransactionRequest.js';
import MempoolTransactionResponse from '../openApi/model/MempoolTransactionResponse.js';
import NetworkRequest from '../openApi/model/NetworkRequest.js';
import TransactionIdentifier from '../openApi/model/TransactionIdentifier.js';
import { RosettaErrorFactory, rosettaPostRouteWithNetwork } from '../rosettaUtils.js';

export default {
	register: (server, db, services) => {
		const blockchainDescriptor = getBlockchainDescriptor(services.config);
		const parser = OperationParser.createFromServices(services, { operationStatus: 'success' });

		// since this is P2P API, simulate an authenticated request
		const getUnconfirmedTransactions = () => services.proxy.fetch('transactions/unconfirmed', jsonObject => jsonObject.entity.data, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({
				challenge: { data: 'AAAAAAAAAA' },
				entity: { hashShortIds: [] }
			})
		});

		server.post('/mempool', rosettaPostRouteWithNetwork(blockchainDescriptor, NetworkRequest, async () => {
			const transactions = await getUnconfirmedTransactions();

			// use signature as unique identifier because hash is not returned by API
			const response = new MempoolResponse();
			response.transaction_identifiers = transactions.map(transaction =>
				new TransactionIdentifier(transaction.signature.toUpperCase()));
			return response;
		}));

		server.post('/mempool/transaction', rosettaPostRouteWithNetwork(
			blockchainDescriptor,
			MempoolTransactionRequest,
			async typedRequest => {
				const transactions = await getUnconfirmedTransactions();

				const transactionSignature = typedRequest.transaction_identifier.hash;
				const isRequestedTransaction = transaction => transaction.signature.toUpperCase() === transactionSignature;
				const transaction = transactions.find(isRequestedTransaction);
				if (!transaction)
					throw RosettaErrorFactory.INVALID_REQUEST_DATA;

				// use signature as unique identifier because hash is not returned by API
				const response = new MempoolTransactionResponse();
				response.transaction = await parser.parseTransactionAsRosettaTransaction(transaction, {
					hash: { data: transaction.signature }
				});
				return response;
			}
		));
	}
};
