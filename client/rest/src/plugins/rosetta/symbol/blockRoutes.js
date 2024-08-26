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
import { getBlockchainDescriptor, stitchBlockTransactions } from './rosettaUtils.js';
import Block from '../openApi/model/Block.js';
import BlockIdentifier from '../openApi/model/BlockIdentifier.js';
import BlockRequest from '../openApi/model/BlockRequest.js';
import BlockResponse from '../openApi/model/BlockResponse.js';
import BlockTransactionRequest from '../openApi/model/BlockTransactionRequest.js';
import BlockTransactionResponse from '../openApi/model/BlockTransactionResponse.js';
import Transaction from '../openApi/model/Transaction.js';
import TransactionIdentifier from '../openApi/model/TransactionIdentifier.js';
import { RosettaErrorFactory, rosettaPostRouteWithNetwork } from '../rosettaUtils.js';

export default {
	register: (server, db, services) => {
		const GENESIS_BLOCK_NUMBER = 1;
		const PAGE_SIZE = 100;

		const blockchainDescriptor = getBlockchainDescriptor(services.config);
		const parser = OperationParser.createFromServices(services, { operationStatus: 'success' });

		const mapBlockStatementsToRosettaTransaction = async blockStatements => {
			// process all statements and receipts
			const statementGroupedOperations = await Promise.all(blockStatements.map(async statement => {
				const receiptGroupedOperations = await Promise.all(statement.statement.receipts.map(async receipt => {
					const { operations } = await parser.parseReceipt(receipt);
					return operations;
				}));

				return [].concat(...receiptGroupedOperations);
			}));

			// flatten and renumber operations
			const operations = [].concat(...statementGroupedOperations);
			operations.forEach((operation, i) => { operation.operation_identifier.index = i; });

			// construct transaction (transaction_identifier.hash is set later)
			return new Transaction(new TransactionIdentifier(undefined), operations);
		};

		server.post('/block', rosettaPostRouteWithNetwork(blockchainDescriptor, BlockRequest, async typedRequest => {
			const height = typedRequest.block_identifier.index;
			const [networkProperties, blockInfo, rosettaTransactions, blockTransaction] = await Promise.all([
				services.proxy.networkProperties(),
				GENESIS_BLOCK_NUMBER === height ? services.proxy.nemesisBlock() : services.proxy.fetch(`blocks/${height}`),
				services.proxy.fetchAll(`transactions/confirmed?height=${height}&embedded=true`, PAGE_SIZE).then(transactions =>
					Promise.all(stitchBlockTransactions(transactions)
						.map(transaction => parser.parseTransactionAsRosettaTransaction(transaction.transaction, transaction.meta)))),
				services.proxy.fetchAll(`statements/transaction?height=${height}`, PAGE_SIZE).then(mapBlockStatementsToRosettaTransaction)
			]);

			blockTransaction.transaction_identifier.hash = blockInfo.meta.hash;

			const response = new BlockResponse();
			response.block = new Block();
			response.block.transactions = rosettaTransactions;
			response.block.transactions.push(blockTransaction);
			response.block.block_identifier = new BlockIdentifier(Number(blockInfo.block.height), blockInfo.meta.hash);
			response.block.parent_block_identifier = GENESIS_BLOCK_NUMBER === height
				? response.block.block_identifier
				: new BlockIdentifier(Number(blockInfo.block.height) - 1, blockInfo.block.previousBlockHash);
			response.block.timestamp = Number(networkProperties.network.epochAdjustment + BigInt(blockInfo.block.timestamp));
			return response;
		}));

		server.post('/block/transaction', rosettaPostRouteWithNetwork(blockchainDescriptor, BlockTransactionRequest, async typedRequest => {
			const height = typedRequest.block_identifier.index;
			const transactionHash = typedRequest.transaction_identifier.hash;
			const transaction = await services.proxy.fetch(`transactions/confirmed/${transactionHash}`);

			if (Number(transaction.meta.height) !== height)
				throw RosettaErrorFactory.INVALID_REQUEST_DATA;

			const response = new BlockTransactionResponse();
			response.transaction = await parser.parseTransactionAsRosettaTransaction(transaction.transaction, transaction.meta);
			return response;
		}));
	}
};
