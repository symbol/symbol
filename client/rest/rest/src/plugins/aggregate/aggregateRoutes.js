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

const routeUtils = require('../../routes/routeUtils');
const catapult = require('catapult-sdk');

const { convert, uint64 } = catapult.utils;
const { PacketType } = catapult.packet;

module.exports = {
	register: (server, db, services) => {
		const parseUint64StringToUint8Buffer = numericString => convert.hexToUint8(uint64.toHex(uint64.fromString(numericString)));
		const parseHexParam = (params, key) => routeUtils.parseArgument(params, key, convert.hexToUint8);

		routeUtils.addPutPacketRoute(
			server,
			services.connections,
			{ routeName: '/transactions/partial', packetType: PacketType.pushPartialTransactions },
			params => parseHexParam(params, 'payload')
		);

		routeUtils.addPutPacketRoute(
			server,
			services.connections,
			{ routeName: '/transactions/cosignature', packetType: PacketType.pushDetachedCosignatures },
			params => Buffer.concat(
				[parseUint64StringToUint8Buffer(params.version)].concat(
					['signerPublicKey', 'signature', 'parentHash'].map(key => parseHexParam(params, key))
				)
			)
		);
	}
};
