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
import Block from './openApi/model/Block.js';
import BlockIdentifier from './openApi/model/BlockIdentifier.js';
import BlockRequest from './openApi/model/BlockRequest.js';
import BlockResponse from './openApi/model/BlockResponse.js';
import BlockTransactionRequest from './openApi/model/BlockTransactionRequest.js';
import BlockTransactionResponse from './openApi/model/BlockTransactionResponse.js';
import RosettaTransaction from './openApi/model/Transaction.js';
import TransactionIdentifier from './openApi/model/TransactionIdentifier.js';
import {
	RosettaErrorFactory, createLookupCurrencyFunction, rosettaPostRouteWithNetwork, stitchBlockTransactions
} from './rosettaUtils.js';
import { NetworkLocator } from 'symbol-sdk';
import { Network } from 'symbol-sdk/symbol';

export default {
	register: (server, db, services) => {
		const networkName = services.config.network.name;
		const network = NetworkLocator.findByName(Network.NETWORKS, networkName);
		const lookupCurrency = createLookupCurrencyFunction(services.proxy);
		const parser = new OperationParser(network, {
			includeFeeOperation: true,
			lookupCurrency,
			resolveAddress: services.proxy.resolveAddress
		});

		server.post('/block', rosettaPostRouteWithNetwork(networkName, BlockRequest, async typedRequest => {
			const height = typedRequest.block_identifier.index;
			const genesisBlockNumber = 1;
			const pageSize = 100;
			const results = await Promise.all([
				genesisBlockNumber === height ? services.proxy.nemesisBlock() : services.proxy.fetch(`blocks/${height}`),
				services.proxy.fetchAll(`transactions/confirmed?height=${height}&embedded=true`, pageSize),
				services.proxy.fetchAll(`statements/transaction?height=${height}`, pageSize)
			]);

			const blockEventsAsRosettaTransaction = async (blockHash, blockStatements) => {
				const result = await Promise.all(blockStatements.map(async statement =>
					Promise.all(statement.statement.receipts.map(async receipt => {
						const receiptOperation = await parser.parseReceipt(receipt);
						return receiptOperation.operations;
					}))));

				const operations = await Promise.all(result.flat(Infinity));
				let id = 0;
				operations.forEach(operation => { operation.operation_identifier.index = id++; }); // fix operation index
				const rosettaTransaction = new RosettaTransaction();
				rosettaTransaction.transaction_identifier = new TransactionIdentifier(blockHash);
				rosettaTransaction.operations = operations;
				return rosettaTransaction;
			};

			const blockInfo = results[0];
			const transactions = results[1];
			const blockStatements = results[2];

			const networkProperties = await services.proxy.networkProperties();
			const epochAdjustment = Number(networkProperties.network.epochAdjustment.slice(0, -1));
			const blockTransaction = await blockEventsAsRosettaTransaction(blockInfo.meta.hash, blockStatements);
			const rosettaTransactions = await Promise.all(stitchBlockTransactions(transactions)
				.map(transaction => parser.parseTransactionAsRosettaTransaction(transaction.transaction, transaction.meta)));

			const response = new BlockResponse();
			response.block = new Block();
			response.block.transactions = rosettaTransactions;
			response.block.transactions.push(blockTransaction);
			response.block.block_identifier = new BlockIdentifier(Number(blockInfo.block.height), blockInfo.meta.hash);
			response.block.parent_block_identifier = genesisBlockNumber === height ? response.block.block_identifier
				: new BlockIdentifier(Number(blockInfo.block.height) - 1, blockInfo.block.previousBlockHash);
			response.block.timestamp = (epochAdjustment * 1000) + Number(blockInfo.block.timestamp); // in milliseconds
			return response;
		}));

		server.post('/block/transaction', rosettaPostRouteWithNetwork(networkName, BlockTransactionRequest, async typedRequest => {
			const height = typedRequest.block_identifier.index;
			const transactionId = typedRequest.transaction_identifier.hash;
			const transaction = await services.proxy.fetch(`transactions/confirmed/${transactionId}`);

			if (Number(transaction.meta.height) !== height)
				throw RosettaErrorFactory.INVALID_REQUEST_DATA;

			const response = new BlockTransactionResponse();
			response.transaction = await parser.parseTransactionAsRosettaTransaction(transaction);
			return response;
		}));
	}
};
