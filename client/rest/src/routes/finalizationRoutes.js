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
import routeUtils from './routeUtils.js';
import catapult from '../catapult-sdk/index.js';
import finalizationProofCodec from '../sockets/finalizationProofCodec.js';
import restifyErrors from 'restify-errors';
import { utils } from 'symbol-sdk';

const packetHeader = catapult.packet.header;
const { PacketType } = catapult.packet;
const { BinaryParser } = catapult.parser;
const { NotFoundError } = restifyErrors;

export default {
	register: (server, db, services) => {
		const { connections } = services;
		const { timeout } = services.config.apiNode;

		const sendRequestAndResponse = (requestPacket, res, next) =>
			connections.singleUse()
				.then(connection => connection.pushPull(requestPacket, timeout))
				.then(packet => {
					const binaryParser = new BinaryParser();
					binaryParser.push(packet.payload);
					const payload = finalizationProofCodec.deserialize(binaryParser);
					if (!payload)
						return next(new NotFoundError());
					res.send({ payload, type: routeResultTypes.finalizationProof, formatter: 'ws' });
					return next();
				});

		server.get('/finalization/proof/epoch/:epoch', (req, res, next) => {
			const epoch = routeUtils.parseArgument(req.params, 'epoch', 'uint');

			const uint32Size = 4;
			const headerBuffer = packetHeader.createBuffer(PacketType.finalizationProofAtEpoch, packetHeader.size + uint32Size);
			const epochBuffer = Buffer.alloc(uint32Size);
			epochBuffer.writeUInt32LE(epoch);
			const packetBuffer = Buffer.concat([headerBuffer, epochBuffer]);

			return sendRequestAndResponse(packetBuffer, res, next);
		});

		server.get('/finalization/proof/height/:height', (req, res, next) => {
			const height = routeUtils.parseArgument(req.params, 'height', 'uint64');

			const uint64Size = 8;
			const headerBuffer = packetHeader.createBuffer(PacketType.finalizationProofAtHeight, packetHeader.size + uint64Size);
			const heightBuffer = Buffer.from(utils.intToBytes(height, 8));
			const packetBuffer = Buffer.concat([headerBuffer, heightBuffer]);

			return sendRequestAndResponse(packetBuffer, res, next);
		});
	}
};
