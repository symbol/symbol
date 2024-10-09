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
import Block from '../openApi/model/Block.js';
import BlockIdentifier from '../openApi/model/BlockIdentifier.js';
import BlockRequest from '../openApi/model/BlockRequest.js';
import BlockResponse from '../openApi/model/BlockResponse.js';
import BlockTransactionRequest from '../openApi/model/BlockTransactionRequest.js';
import BlockTransactionResponse from '../openApi/model/BlockTransactionResponse.js';
import Transaction from '../openApi/model/Transaction.js';
import TransactionIdentifier from '../openApi/model/TransactionIdentifier.js';
import { RosettaErrorFactory, rosettaPostRouteWithNetwork } from '../rosettaUtils.js';
import { NetworkLocator } from 'symbol-sdk';
import { Network, NetworkTimestamp } from 'symbol-sdk/nem';

export default {
	register: (server, db, services) => {
		const GENESIS_BLOCK_NUMBER = 1;

		const blockchainDescriptor = getBlockchainDescriptor(services.config);
		const network = NetworkLocator.findByName(Network.NETWORKS, blockchainDescriptor.network);
		const parser = OperationParser.createFromServices(services, { operationStatus: 'success' });

		const createBlockTransaction = async blockInfo => {
			const { operations } = await parser.parseBlock(blockInfo);
			return new Transaction(new TransactionIdentifier(blockInfo.hash), operations);
		};

		server.post('/block', rosettaPostRouteWithNetwork(blockchainDescriptor, BlockRequest, async typedRequest => {
			const height = typedRequest.block_identifier.index;
			const blockInfo = await services.proxy.localBlockAtHeight(height);
			const rosettaTransactions = await Promise.all(blockInfo.txes.map(transaction => parser.parseTransactionAsRosettaTransaction(
				transaction.tx,
				{
					hash: { data: transaction.hash },
					height
				}
			)));
			const blockTransaction = await createBlockTransaction(blockInfo);

			const calculateBlockTimestamp = timestamp => Number(network.toDatetime(new NetworkTimestamp(timestamp)).getTime());

			const response = new BlockResponse();
			response.block = new Block();
			response.block.transactions = rosettaTransactions;
			response.block.transactions.push(blockTransaction);
			response.block.block_identifier = new BlockIdentifier(Number(blockInfo.block.height), blockInfo.hash);
			response.block.parent_block_identifier = GENESIS_BLOCK_NUMBER === height
				? response.block.block_identifier
				: new BlockIdentifier(Number(blockInfo.block.height) - 1, blockInfo.block.prevBlockHash.data.toUpperCase());
			response.block.timestamp = calculateBlockTimestamp(blockInfo.block.timeStamp);
			return response;
		}));

		server.post('/block/transaction', rosettaPostRouteWithNetwork(blockchainDescriptor, BlockTransactionRequest, async typedRequest => {
			const height = typedRequest.block_identifier.index;
			const transactionHash = typedRequest.transaction_identifier.hash;
			const transaction = await services.proxy.fetch(`transaction/get?hash=${transactionHash}`);

			if (undefined === transaction.meta || Number(transaction.meta.height) !== height)
				throw RosettaErrorFactory.INVALID_REQUEST_DATA;

			const response = new BlockTransactionResponse();
			response.transaction = await parser.parseTransactionAsRosettaTransaction(transaction.transaction, transaction.meta);
			return response;
		}));
	}
};
