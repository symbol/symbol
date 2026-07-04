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

import multisigUtils from './multisigUtils.js';
import catapult from '../../catapult-sdk/index.js';
import merkleUtils from '../../routes/merkleUtils.js';
import routeUtils from '../../routes/routeUtils.js';

const { PacketType } = catapult.packet;

export default {
	register: (server, db, services) => {
		server.get('/account/:address/multisig', async (request, reply) => {
			const accountAddress = routeUtils.parseArgument(request.params, 'address', 'address');

			const result = await db.multisigsByAddresses([accountAddress]);
			return reply.send(routeUtils.createSender('multisigEntry').sendOne(request.params.address)(result));
		});

		server.get('/account/:address/multisig/merkle', async (request, reply) => {
			const accountAddress = routeUtils.parseArgument(request.params, 'address', 'address');
			const state = PacketType.multisigStatePath;
			const response = await merkleUtils.requestTree(services, state, accountAddress);
			return reply.send(response);
		});

		server.get('/account/:address/multisig/graph', async (request, reply) => {
			const accountAddress = routeUtils.parseArgument(request.params, 'address', 'address');
			const response = await multisigUtils.getMultisigGraph(db, accountAddress);
			const sender = routeUtils.createSender('multisigGraph');
			return reply.send(undefined === response
				? sender.sendOne(request.params.address)(response)
				: sender.sendArray(request.params.address)(response));
		});
	}
};
