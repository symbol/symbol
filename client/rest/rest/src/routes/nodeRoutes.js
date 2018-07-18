/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const catapult = require('catapult-sdk');
const routeResultTypes = require('./routeResultTypes');
const nodeInfoCodec = require('../sockets/nodeInfoCodec');
const nodeTimeCodec = require('../sockets/nodeTimeCodec');

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

		server.get('/node/info', (req, res, next) => {
			const packetBuffer = packetHeader.createBuffer(PacketType.nodeDiscoveryPullPing, packetHeader.size);
			return connections.singleUse()
				.then(connection => connection.pushPull(packetBuffer, timeout))
				.then(packet => {
					res.send(buildResponse(packet, nodeInfoCodec, routeResultTypes.nodeInfo));
					next();
				});
		});

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
