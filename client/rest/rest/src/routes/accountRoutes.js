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

const catapult = require('catapult-sdk');
const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');
const AccountType = require('../plugins/AccountType');
const errors = require('../server/errors');

const { convert } = catapult.utils;

module.exports = {
	register: (server, db, services) => {
		const transactionSender = routeUtils.createSender(routeResultTypes.transfer);

		server.get('/account/:accountId', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const sender = routeUtils.createSender(routeResultTypes.account);
			return db.accountsByIds([{ [type]: accountId }])
				.then(sender.sendOne(req.params.accountId, res, next));
		});

		server.post('/account', (req, res, next) => {
			if (req.params.publicKeys && req.params.addresses)
				throw errors.createInvalidArgumentError('publicKeys and addresses cannot both be provided');

			const idOptions = Array.isArray(req.params.publicKeys)
				? { keyName: 'publicKeys', parserName: 'publicKey', type: AccountType.publicKey }
				: { keyName: 'addresses', parserName: 'address', type: AccountType.address };

			const accountIds = routeUtils.parseArgumentAsArray(req.params, idOptions.keyName, idOptions.parserName);
			const sender = routeUtils.createSender(routeResultTypes.account);

			return db.accountsByIds(accountIds.map(accountId => ({ [idOptions.type]: accountId })))
				.then(sender.sendArray(idOptions.keyName, res, next));
		});


		const transactionStates = [
			{ dbPostfix: 'All', routePostfix: '' },
			{ dbPostfix: 'Incoming', routePostfix: '/incoming' },
			{ dbPostfix: 'Outgoing', routePostfix: '/outgoing' },
			{ dbPostfix: 'Unconfirmed', routePostfix: '/unconfirmed' }
		];

		transactionStates.concat(services.config.transactionStates).forEach(state => {
			server.get(`/account/:publicKey/transactions${state.routePostfix}`, (req, res, next) => {
				const publicKey = routeUtils.parseArgument(req.params, 'publicKey', convert.hexToUint8);
				const pagingOptions = routeUtils.parsePagingArguments(req.params);
				return db[`accountTransactions${state.dbPostfix}`](publicKey, pagingOptions.id, pagingOptions.pageSize)
					.then(transactionSender.sendArray('publicKey', res, next));
			});
		});
	}
};
