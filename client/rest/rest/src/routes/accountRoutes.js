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

const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');
const AccountType = require('../plugins/AccountType');
const errors = require('../server/errors');

module.exports = {
	register: (server, db, services) => {
		const sender = routeUtils.createSender(routeResultTypes.account);

		server.get('/accounts', (req, res, next) => {
			const address = req.params.address ? routeUtils.parseArgument(req.params, 'address', 'address') : undefined;
			const mosaicId = req.params.mosaicId ? routeUtils.parseArgument(req.params, 'mosaicId', 'uint64hex') : undefined;

			const offsetParsers = {
				id: 'objectId',
				balance: 'uint64'
			};
			const options = routeUtils.parsePaginationArguments(req.params, services.config.pageSize, offsetParsers);

			if ('balance' === options.sortField && !mosaicId)
				throw errors.createInvalidArgumentError('mosaicId must be provided when sorting by balance');

			return db.accounts(address, mosaicId, options)
				.then(result => sender.sendPage(res, next)(result));
		});

		server.get('/accounts/:accountId', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			return db.accountsByIds([{ [type]: accountId }])
				.then(sender.sendOne(req.params.accountId, res, next));
		});

		server.post('/accounts', (req, res, next) => {
			if (req.params.publicKeys && req.params.addresses)
				throw errors.createInvalidArgumentError('publicKeys and addresses cannot both be provided');

			const idOptions = Array.isArray(req.params.publicKeys)
				? { keyName: 'publicKeys', parserName: 'publicKey', type: AccountType.publicKey }
				: { keyName: 'addresses', parserName: 'address', type: AccountType.address };

			const accountIds = routeUtils.parseArgumentAsArray(req.params, idOptions.keyName, idOptions.parserName);

			return db.accountsByIds(accountIds.map(accountId => ({ [idOptions.type]: accountId })))
				.then(sender.sendArray(idOptions.keyName, res, next));
		});
	}
};
