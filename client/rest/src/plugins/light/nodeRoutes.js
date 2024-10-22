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

import catapult from '../../catapult-sdk/index.js';
import routeResultTypes from '../../routes/routeResultTypes.js';
import nodeInfoCodec from '../../sockets/nodeInfoCodec.js';
import { utils } from 'symbol-sdk';
import fs from 'fs';
import path from 'path';

const packetHeader = catapult.packet.header;
const { PacketType } = catapult.packet;
const { BinaryParser } = catapult.parser;

const restVersion = JSON.parse(fs.readFileSync(path.resolve(import.meta.dirname, '../../../package.json'), 'UTF-8')).version;

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

		server.get('/node/info', (req, res, next) => {
			const packetBuffer = packetHeader.createBuffer(
				PacketType.nodeDiscoveryPullPing,
				packetHeader.size
			);

			return connections
				.singleUse()
				.then(connection => connection.pushPull(packetBuffer, timeout))
				.then(packet => {
					const response = buildResponse(packet, nodeInfoCodec, routeResultTypes.nodeInfo);
					response.payload.nodePublicKey = services.config.apiNode.nodePublicKey;
					res.send(response);
					next();
				});
		});

		server.get('/node/server', (req, res, next) => {
			const { deployment } = services.config;
			res.send({
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
			return next();
		});

		server.get('/node/unlockedaccount', (req, res, next) => {
			const headerBuffer = packetHeader.createBuffer(
				PacketType.unlockedAccount,
				packetHeader.size
			);
			const packetBuffer = headerBuffer;
			return connections
				.singleUse()
				.then(connection => connection.pushPull(packetBuffer, timeout))
				.then(packet => {
					const unlockedKeys = utils.uint8ToHex(packet.payload)
						.match(/.{1,64}/g);
					res.send({ unlockedAccount: !unlockedKeys ? [] : unlockedKeys });
					next();
				});
		});
	}
};
