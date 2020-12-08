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
		const sender = routeUtils.createSender('secretLockInfo');

		server.get('/account/:address/lock/secret', (req, res, next) => {
			const { params } = req;
			const accountAddresses = params.address ? [routeUtils.parseArgument(params, 'address', 'address')] : [];
			const secret = params.secret ? routeUtils.parseArgument(params, 'secret', 'hash256') : undefined;
			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });
			return db.secretLocks(accountAddresses, secret, options)
				.then(result => sender.sendPage(res, next)(result));
		});

		server.get('/lock/secret', (req, res, next) => {
			const { params } = req;
			const accountAddresses = params.address ? [routeUtils.parseArgument(params, 'address', 'address')] : [];
			const secret = params.secret ? routeUtils.parseArgument(params, 'secret', 'hash256') : undefined;
			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });
			return db.secretLocks(accountAddresses, secret, options)
				.then(result => sender.sendPage(res, next)(result));
		});

		routeUtils.addGetPostDocumentRoutes(
			server,
			sender,
			{ base: '/lock/secret', singular: 'compositeHash', plural: 'compositeHashes' },
			params => db.secretLocksByCompositeHash(params),
			routeUtils.namedParserMap.hash256
		);

		server.get('/lock/secret/:compositeHash/merkle', (req, res, next) => {
			const compositeHash = routeUtils.parseArgument(req.params, 'compositeHash', 'hash256');
			const state = PacketType.secretLockStatePath;
			return merkleUtils.requestTree(services, state,
				compositeHash).then(response => {
				res.send(response);
				next();
			});
		});
	}
};
