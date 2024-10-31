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
import chainInfoCodec from '../../sockets/chainInfoCodec.js';
import finalizedBlockCodec from '../../sockets/finalizedBlockCodec.js';
import routeResultTypes from '../routeResultTypes.js';

const packetHeader = catapult.packet.header;
const { PacketType } = catapult.packet;
const { BinaryParser } = catapult.parser;

export default {
	register: (server, db, services) => {
		const { connections } = services;
		const { timeout } = services.config.apiNode;

		const fetchAndParse = async (packetBuffer, codec) => {
			const connection = await connections.singleUse();
			const packet = await connection.pushPull(packetBuffer, timeout);

			const binaryParserInfo = new BinaryParser();
			binaryParserInfo.push(packet.payload);

			return codec.deserialize(binaryParserInfo);
		};

		const createPacketBuffer = packetType => packetHeader.createBuffer(packetType, packetHeader.size);

		server.get('/chain/info', async (req, res, next) => {
			const packetBufferChainStatistics = createPacketBuffer(PacketType.chainStatistics);
			const packetBufferFinalizationStatistics = createPacketBuffer(PacketType.finalizationStatistics);

			const [chainInfo, latestFinalizedBlock] = await Promise.all([
				fetchAndParse(packetBufferChainStatistics, chainInfoCodec),
				fetchAndParse(packetBufferFinalizationStatistics, finalizedBlockCodec)
			]);

			const response = {
				payload: {
					...chainInfo,
					latestFinalizedBlock
				},
				type: routeResultTypes.chainInfo,
				formatter: 'ws'
			};

			res.send(response);
			next();
		});
	}
};
