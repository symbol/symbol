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

import Allow from './openApi/model/Allow.js';
import BlockIdentifier from './openApi/model/BlockIdentifier.js';
import NetworkIdentifier from './openApi/model/NetworkIdentifier.js';
import NetworkListResponse from './openApi/model/NetworkListResponse.js';
import NetworkOptionsResponse from './openApi/model/NetworkOptionsResponse.js';
import NetworkRequest from './openApi/model/NetworkRequest.js';
import NetworkStatusResponse from './openApi/model/NetworkStatusResponse.js';
import RosettaNodePeer from './openApi/model/Peer.js';
import Version from './openApi/model/Version.js';
import {
	RosettaErrorFactory, operationStatus, operationTypes, rosettaPostRouteWithNetwork
} from './rosettaUtils.js';
import { sendJson } from '../../routes/simpleSend.js';

export default {
	register: (server, db, services) => {
		const networkName = services.config.network.name;

		server.post('/network/list', (req, res, next) => {
			const networkIdentifier = new NetworkIdentifier('Symbol', networkName);
			const networkListResponse = new NetworkListResponse([networkIdentifier]);
			return sendJson(res, next)(networkListResponse);
		});

		server.post('/network/options', rosettaPostRouteWithNetwork(networkName, NetworkRequest, async () => {
			const getErrorsFromFactoryClass = factory => {
				const defaultClassPropertyNames = Object.getOwnPropertyNames(class C {});
				const errorNames = Object.getOwnPropertyNames(factory)
					.filter(name => !defaultClassPropertyNames.includes(name));
				return errorNames.map(name => factory[name].apiError);
			};

			const formatNodeVersion = rawNodeVersion => [
				(rawNodeVersion >> 24) & 0xFF,
				(rawNodeVersion >> 16) & 0xFF,
				(rawNodeVersion >> 8) & 0xFF,
				rawNodeVersion & 0xFF
			].join('.');

			const nodeInfo = await services.proxy.nodeInfo();
			const nodeVersion = formatNodeVersion(nodeInfo.version);

			const version = new Version();
			version.rosetta_version = '1.4.13';
			version.node_version = nodeVersion;

			const allow = new Allow();
			allow.balance_exemptions = [];
			allow.call_methods = [];
			allow.errors = getErrorsFromFactoryClass(RosettaErrorFactory);
			allow.historical_balance_lookup = false;
			allow.mempool_coins = false;
			allow.operation_statuses = Object.values(operationStatus);
			allow.operation_types = Object.values(operationTypes);

			return new NetworkOptionsResponse(version, allow);
		}));

		server.post('/network/status', rosettaPostRouteWithNetwork(networkName, NetworkRequest, async () => {
			const getBlockByNumber = async blockNumber => services.proxy.fetch(`blocks/${blockNumber}`);

			const results = await Promise.all([
				services.proxy.fetch('chain/info').then(result => getBlockByNumber(result.height)),
				services.proxy.fetch('node/peers')
			]);

			const currentBlock = results[0];
			const genesisBlock = await services.proxy.nemesisBlock();
			const peers = results[1].map(nodePeer => new RosettaNodePeer(nodePeer.publicKey));
			const networkProperties = await services.proxy.networkProperties();
			const epochAdjustment = Number(networkProperties.network.epochAdjustment.slice(0, -1));
			const currentBlockTimestamp = (epochAdjustment * 1000) + Number(currentBlock.block.timestamp); // in milliseconds

			const currentBlockIdentifier = new BlockIdentifier();
			currentBlockIdentifier.hash = currentBlock.meta.hash;
			currentBlockIdentifier.index = Number(currentBlock.block.height);

			const genesisBlockIdentifier = new BlockIdentifier();
			genesisBlockIdentifier.hash = genesisBlock.meta.hash;
			genesisBlockIdentifier.index = Number(genesisBlock.block.height);

			const networkStatusResponse = new NetworkStatusResponse();
			networkStatusResponse.current_block_identifier = currentBlockIdentifier;
			networkStatusResponse.current_block_timestamp = currentBlockTimestamp;
			networkStatusResponse.genesis_block_identifier = genesisBlockIdentifier;
			networkStatusResponse.peers = peers;
			return networkStatusResponse;
		}));
	}
};
