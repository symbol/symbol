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

import routeResultTypes from './routeResultTypes.js';
import catapult from '../catapult-sdk/index.js';
import nodeInfoCodec from '../sockets/nodeInfoCodec.js';
import nodePeersCodec from '../sockets/nodePeersCodec.js';
import nodeTimeCodec from '../sockets/nodeTimeCodec.js';
import { utils } from 'symbol-sdk';
import fs from 'fs';
import path from 'path';

const packetHeader = catapult.packet.header;
const { PacketType } = catapult.packet;
const { BinaryParser } = catapult.parser;

const restVersion = JSON.parse(fs.readFileSync(path.resolve(import.meta.dirname, '../../package.json'), 'UTF-8')).version;

const buildResponse = (packet, codec, resultType) => {
	const binaryParser = new BinaryParser();
	binaryParser.push(packet.payload);
	return {
		payload: codec.deserialize(binaryParser),
		type: resultType,
		formatter: 'ws'
	};
};

export default {
	register: (server, db, services) => {
		const { connections } = services;
		const { timeout } = services.config.apiNode;

		server.get('/node/health', async (request, reply) => {
			const parseNodeInfoPacket = packet => {
				const binaryParser = new BinaryParser();
				binaryParser.push(packet.payload);
				return nodeInfoCodec.deserialize(binaryParser);
			};

			const ServiceStatus = Object.freeze({
				up: 'up',
				down: 'down'
			});

			// Check apiNode status
			const packetBuffer = packetHeader.createBuffer(
				PacketType.nodeDiscoveryPullPing,
				packetHeader.size
			);
			const apiNodeStatusPromise = services.connections
				.singleUse()
				.then(connection =>
					connection.pushPull(packetBuffer, services.config.apiNode.timeout))
				.then(packet => parseNodeInfoPacket(packet));

			const dbStatusPromise = db.client.db().admin().ping();

			const results = await Promise.allSettled([dbStatusPromise, apiNodeStatusPromise]);
			const statusCode = results.some(result => 'fulfilled' !== result.status) ? 503 : 200;
			const checkResult = result => ('fulfilled' === result.status ? ServiceStatus.up : ServiceStatus.down);

			reply.code(statusCode);
			return reply.send({
				payload: {
					status: {
						apiNode: checkResult(results[1]),
						db: checkResult(results[0])
					}
				},
				type: routeResultTypes.nodeHealth
			});
		});

		server.get('/node/info', async (request, reply) => {
			const packetBuffer = packetHeader.createBuffer(
				PacketType.nodeDiscoveryPullPing,
				packetHeader.size
			);
			const packet = await connections.singleUse().then(connection => connection.pushPull(packetBuffer, timeout));
			const response = buildResponse(packet, nodeInfoCodec, routeResultTypes.nodeInfo);
			response.payload.nodePublicKey = services.config.apiNode.nodePublicKey;
			return reply.send(response);
		});

		server.get('/node/peers', async (request, reply) => {
			const packetBuffer = packetHeader.createBuffer(
				PacketType.nodeDiscoveryPullPeers,
				packetHeader.size
			);
			const packet = await connections.singleUse().then(connection => connection.pushPull(packetBuffer, timeout));
			return reply.send(buildResponse(packet, nodePeersCodec, routeResultTypes.nodeInfo));
		});

		server.get('/node/server', async (request, reply) => {
			const { deployment } = services.config;
			return reply.send({
				payload: {
					serverInfo: {
						restVersion,
						deployment: {
							deploymentTool: deployment && deployment.deploymentTool ? deployment.deploymentTool : 'N/A',
							deploymentToolVersion: deployment && deployment.deploymentToolVersion
								? deployment.deploymentToolVersion
								: 'N/A',
							lastUpdatedDate: deployment && deployment.lastUpdatedDate ? deployment.lastUpdatedDate : 'N/A'
						}
					}
				},
				type: routeResultTypes.serverInfo
			});
		});

		server.get('/node/storage', async (request, reply) => {
			const storageInfo = await db.storageInfo();
			return reply.send({ payload: storageInfo, type: routeResultTypes.storageInfo });
		});

		server.get('/node/time', async (request, reply) => {
			const packetBuffer = packetHeader.createBuffer(
				PacketType.timeSyncNodeTime,
				packetHeader.size
			);
			const packet = await connections.singleUse().then(connection => connection.pushPull(packetBuffer, timeout));
			return reply.send(buildResponse(packet, nodeTimeCodec, routeResultTypes.nodeTime));
		});

		server.get('/node/unlockedaccount', async (request, reply) => {
			const headerBuffer = packetHeader.createBuffer(
				PacketType.unlockedAccount,
				packetHeader.size
			);
			const packetBuffer = headerBuffer;
			const packet = await connections.singleUse().then(connection => connection.pushPull(packetBuffer, timeout));
			const unlockedKeys = utils.uint8ToHex(packet.payload).match(/.{1,64}/g);
			return reply.send({ unlockedAccount: !unlockedKeys ? [] : unlockedKeys });
		});

		server.get('/node/metadata', async (request, reply) => reply.send(services.config.nodeMetadata));
	}
};
