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

import { getBlockchainDescriptor } from './rosettaUtils.js';
import { sendJson } from '../../../routes/simpleSend.js';
import Allow from '../openApi/model/Allow.js';
import BlockIdentifier from '../openApi/model/BlockIdentifier.js';
import NetworkIdentifier from '../openApi/model/NetworkIdentifier.js';
import NetworkListResponse from '../openApi/model/NetworkListResponse.js';
import NetworkOptionsResponse from '../openApi/model/NetworkOptionsResponse.js';
import NetworkRequest from '../openApi/model/NetworkRequest.js';
import NetworkStatusResponse from '../openApi/model/NetworkStatusResponse.js';
import OperationStatus from '../openApi/model/OperationStatus.js';
import Peer from '../openApi/model/Peer.js';
import Version from '../openApi/model/Version.js';
import { RosettaErrorFactory, rosettaPostRouteWithNetwork } from '../rosettaUtils.js';
import { NetworkLocator } from 'symbol-sdk';
import { Network, NetworkTimestamp } from 'symbol-sdk/nem';

export default {
	register: (server, db, services) => {
		const blockchainDescriptor = getBlockchainDescriptor(services.config);

		server.post('/network/list', (req, res, next) => {
			const networkIdentifier = new NetworkIdentifier(blockchainDescriptor.blockchain, blockchainDescriptor.network);
			const networkListResponse = new NetworkListResponse([networkIdentifier]);
			return sendJson(res, next)(networkListResponse);
		});

		server.post('/network/options', rosettaPostRouteWithNetwork(blockchainDescriptor, NetworkRequest, async () => {
			const getErrorsFromFactoryClass = factory => {
				const defaultClassPropertyNames = Object.getOwnPropertyNames(class C {});
				const errorNames = Object.getOwnPropertyNames(factory)
					.filter(name => !defaultClassPropertyNames.includes(name));
				return errorNames.map(name => factory[name].apiError);
			};

			const nodeInfo = await services.proxy.fetch('node/info');
			const version = new Version();
			version.rosetta_version = '1.4.13';
			version.node_version = nodeInfo.metaData.version;

			const isHistoricalNode = features => 2 === (features & 0x2);

			const allow = new Allow();
			allow.balance_exemptions = [];
			allow.call_methods = [];
			allow.errors = getErrorsFromFactoryClass(RosettaErrorFactory);
			allow.historical_balance_lookup = isHistoricalNode(nodeInfo.metaData.features);
			allow.mempool_coins = false;
			allow.operation_statuses = [new OperationStatus('success', true)];
			allow.operation_types = ['cosign', 'fee', 'multisig', 'transfer'];

			return new NetworkOptionsResponse(version, allow);
		}));

		server.post('/network/status', rosettaPostRouteWithNetwork(blockchainDescriptor, NetworkRequest, async () => {
			const [currentBlock, peers, genesisBlock] = await Promise.all([
				services.proxy.fetch('chain/height').then(info => services.proxy.localBlockAtHeight(info.height)),
				services.proxy.fetch(
					'node/peer-list/reachable',
					jsonObject => jsonObject.data
				).then(nodes => nodes.map(node => new Peer(node.identity['public-key']))),
				services.proxy.localBlockAtHeight(1)
			]);

			const network = NetworkLocator.findByName(Network.NETWORKS, blockchainDescriptor.network);
			const calculateUnixTime = timestamp => Number(network.toDatetime(new NetworkTimestamp(timestamp)).getTime());

			const currentBlockTimestamp = calculateUnixTime(currentBlock.block.timeStamp);
			const blockToBlockIdentifier = block => new BlockIdentifier(Number(block.block.height), block.hash);
			const currentBlockIdentifier = blockToBlockIdentifier(currentBlock);
			const genesisBlockIdentifier = blockToBlockIdentifier(genesisBlock);

			const networkStatusResponse = new NetworkStatusResponse();
			networkStatusResponse.current_block_identifier = currentBlockIdentifier;
			networkStatusResponse.current_block_timestamp = currentBlockTimestamp;
			networkStatusResponse.genesis_block_identifier = genesisBlockIdentifier;
			networkStatusResponse.peers = peers;
			return networkStatusResponse;
		}));
	}
};
