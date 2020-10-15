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

const routeResultTypes = require('./routeResultTypes');
const { version: sdkVersion } = require('../../../catapult-sdk/package.json');
const { version: restVersion } = require('../../package.json');
const nodeInfoCodec = require('../sockets/nodeInfoCodec');
const nodePeersCodec = require('../sockets/nodePeersCodec');
const nodeTimeCodec = require('../sockets/nodeTimeCodec');
const catapult = require('catapult-sdk');

const packetHeader = catapult.packet.header;
const { PacketType } = catapult.packet;
const { BinaryParser } = catapult.parser;

const buildResponse = (packet, codec, resultType) => {
	const binaryParser = new BinaryParser();
	binaryParser.push(packet.payload);
	return { payload: codec.deserialize(binaryParser), type: resultType, formatter: 'ws' };
};

module.exports = {
	register: (server, db, services) => {
		const { connections } = services;
		const { timeout } = services.config.apiNode;

		server.get('/node/health', (req, res, next) => {
			const parseNodeInfoPacket = packet => {
				const binaryParser = new BinaryParser();
				binaryParser.push(packet.payload);
				return nodeInfoCodec.deserialize(binaryParser);
			};

			const ServiceStatus = Object.freeze({
				up: 'up',
				down: 'down'
			});

			// Check database status
			const dbStatusPromise = new Promise((resolve, reject) => (db.database.serverConfig.isConnected() ? resolve() : reject()));

			// Check apiNode status
			const packetBuffer = packetHeader.createBuffer(PacketType.nodeDiscoveryPullPing, packetHeader.size);
			const apiNodeStatusPromise = services.connections.singleUse()
				.then(connection => connection.pushPull(packetBuffer, services.config.apiNode.timeout))
				.then(packet => parseNodeInfoPacket(packet));

			return Promise.allSettled([dbStatusPromise, apiNodeStatusPromise])
				.then(results => {
					const statusCode = results.some(result => 'fulfilled' !== result.status) ? 503 : 200;
					const checkResult = result => ('fulfilled' === result.status ? ServiceStatus.up : ServiceStatus.down);

					res.status(statusCode);
					res.send({
						payload: {
							status: {
								apiNode: checkResult(results[1]),
								db: checkResult(results[0])
							}
						},
						type: routeResultTypes.nodeHealth
					});
					next();
				});
		});

		server.get('/node/info', (req, res, next) => {
			const packetBuffer = packetHeader.createBuffer(PacketType.nodeDiscoveryPullPing, packetHeader.size);
			return connections.singleUse()
				.then(connection => connection.pushPull(packetBuffer, timeout))
				.then(packet => {
					res.send(buildResponse(packet, nodeInfoCodec, routeResultTypes.nodeInfo));
					next();
				});
		});

		server.get('/node/peers', (req, res, next) => {
			const packetBuffer = packetHeader.createBuffer(PacketType.nodeDiscoveryPullPeers, packetHeader.size);
			return connections.singleUse()
				.then(connection => connection.pushPull(packetBuffer, timeout))
				.then(packet => {
					res.send(buildResponse(packet, nodePeersCodec, routeResultTypes.nodeInfo));
					next();
				});
		});

		server.get('/node/server', (req, res, next) => {
			res.send({
				payload: {
					serverInfo: {
						restVersion,
						sdkVersion
					}
				},
				type: routeResultTypes.serverInfo
			});
			return next();
		});

		server.get('/node/storage', (req, res, next) =>
			db.storageInfo().then(storageInfo => {
				res.send({ payload: storageInfo, type: routeResultTypes.storageInfo });
				next();
			}));

		server.get('/node/time', (req, res, next) => {
			const packetBuffer = packetHeader.createBuffer(PacketType.timeSyncNodeTime, packetHeader.size);
			return connections.singleUse()
				.then(connection => connection.pushPull(packetBuffer, timeout))
				.then(packet => {
					res.send(buildResponse(packet, nodeTimeCodec, routeResultTypes.nodeTime));
					next();
				});
		});
	}
};
