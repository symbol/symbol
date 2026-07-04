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
import merkleUtils from '../../routes/merkleUtils.js';
import routeUtils from '../../routes/routeUtils.js';

const { PacketType } = catapult.packet;

export default {
	register: (server, db, services) => {
		const sender = routeUtils.createSender('hashLockInfo');

		server.get('/account/:address/lock/hash', async (request, reply) => {
			const accountAddress = routeUtils.parseArgument(request.params, 'address', 'address');
			const options = routeUtils.parsePaginationArguments(request.params, services.config.pageSize, { id: 'objectId' });
			const result = await db.hashLocks([accountAddress], options);
			return reply.send(sender.sendPage()(result));
		});

		// Search
		server.get('/lock/hash', async (request, reply) => {
			const accountAddresses = request.params.address ? [routeUtils.parseArgument(request.params, 'address', 'address')] : [];
			const options = routeUtils.parsePaginationArguments(request.params, services.config.pageSize, { id: 'objectId' });
			const result = await db.hashLocks(accountAddresses, options);
			return reply.send(sender.sendPage()(result));
		});

		// Get by ids
		routeUtils.addGetPostDocumentRoutes(
			server,
			sender,
			{ base: '/lock/hash', singular: 'hash', plural: 'hashes' },
			params => db.hashLockByHash(params),
			routeUtils.namedParserMap.hash256
		);

		// Merkle
		server.get('/lock/hash/:hash/merkle', async (request, reply) => {
			const hash = routeUtils.parseArgument(request.params, 'hash', 'hash256');
			const state = PacketType.hashLockStatePath;
			const response = await merkleUtils.requestTree(services, state, hash);
			return reply.send(response);
		});
	}
};
