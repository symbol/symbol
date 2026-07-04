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
		const sender = routeUtils.createSender('secretLockInfo');

		server.get('/account/:address/lock/secret', async (request, reply) => {
			const { params } = request;
			const accountAddresses = params.address ? [routeUtils.parseArgument(params, 'address', 'address')] : [];
			const secret = params.secret ? routeUtils.parseArgument(params, 'secret', 'hash256') : undefined;
			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });
			const result = await db.secretLocks(accountAddresses, secret, options);
			return reply.send(sender.sendPage()(result));
		});

		server.get('/lock/secret', async (request, reply) => {
			const { params } = request;
			const accountAddresses = params.address ? [routeUtils.parseArgument(params, 'address', 'address')] : [];
			const secret = params.secret ? routeUtils.parseArgument(params, 'secret', 'hash256') : undefined;
			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });
			const result = await db.secretLocks(accountAddresses, secret, options);
			return reply.send(sender.sendPage()(result));
		});

		routeUtils.addGetPostDocumentRoutes(
			server,
			sender,
			{ base: '/lock/secret', singular: 'compositeHash', plural: 'compositeHashes' },
			params => db.secretLocksByCompositeHash(params),
			routeUtils.namedParserMap.hash256
		);

		server.get('/lock/secret/:compositeHash/merkle', async (request, reply) => {
			const compositeHash = routeUtils.parseArgument(request.params, 'compositeHash', 'hash256');
			const state = PacketType.secretLockStatePath;
			const response = await merkleUtils.requestTree(services, state, compositeHash);
			return reply.send(response);
		});
	}
};
