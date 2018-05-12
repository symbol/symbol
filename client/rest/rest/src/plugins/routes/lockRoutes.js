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

const routeUtils = require('../../routes/routeUtils');

module.exports = {
	register: (server, db) => {
		server.get('/account/:accountId/lock/hash', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const pagingOptions = routeUtils.parsePagingArguments(req.params);

			return db.hashLocksByAccounts(type, [accountId], pagingOptions.id, pagingOptions.pageSize)
				.then(routeUtils.createSender('hashLockInfo').sendArray('accountId', res, next));
		});

		server.get('/account/:accountId/lock/secret', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const pagingOptions = routeUtils.parsePagingArguments(req.params);

			return db.secretLocksByAccounts(type, [accountId], pagingOptions.id, pagingOptions.pageSize)
				.then(routeUtils.createSender('secretLockInfo').sendArray('accountId', res, next));
		});

		server.get('/lock/hash/:hash', (req, res, next) => {
			const hash = routeUtils.parseArgument(req.params, 'hash', 'hash256');

			return db.hashLockByHash(hash)
				.then(routeUtils.createSender('hashLockInfo').sendOne(req.params.hash, res, next));
		});

		server.get('/lock/secret/:secret', (req, res, next) => {
			const secret = routeUtils.parseArgument(req.params, 'secret', 'hash512');

			return db.secretLockBySecret(secret)
				.then(routeUtils.createSender('secretLockInfo').sendOne(req.params.secret, res, next));
		});
	}
};
