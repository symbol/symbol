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

const merkleUtils = require('../../routes/merkleUtils');
const routeUtils = require('../../routes/routeUtils');
const catapult = require('catapult-sdk');

const { PacketType } = catapult.packet;

module.exports = {
	register: (server, db, services) => {
		const sender = routeUtils.createSender('hashLockInfo');

		server.get('/account/:address/lock/hash', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			const options = routeUtils.parsePaginationArguments(req.params, services.config.pageSize, { id: 'objectId' });
			return db.hashLocks([accountAddress], options)
				.then(result => sender.sendPage(res, next)(result));
		});

		// Search
		server.get('/lock/hash', (req, res, next) => {
			const accountAddresses = req.params.address ? [routeUtils.parseArgument(req.params, 'address', 'address')] : [];
			const options = routeUtils.parsePaginationArguments(req.params, services.config.pageSize, { id: 'objectId' });
			return db.hashLocks(accountAddresses, options)
				.then(result => sender.sendPage(res, next)(result));
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
		server.get('/lock/hash/:hash/merkle', (req, res, next) => {
			const hash = routeUtils.parseArgument(req.params, 'hash', 'hash256');
			const state = PacketType.hashLockStatePath;
			return merkleUtils.requestTree(services, state,
				hash).then(response => {
				res.send(response);
				next();
			});
		});
	}
};
