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

const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');
const AccountType = require('../plugins/AccountType');
const errors = require('../server/errors');
const catapult = require('catapult-sdk');

const { address, networkInfo } = catapult.model;

module.exports = {
	register: (server, db, services) => {
		const transactionSender = routeUtils.createSender(routeResultTypes.transaction);

		const accountIdToPublicKey = (type, accountId) => {
			if (AccountType.publicKey === type)
				return Promise.resolve(accountId);

			return routeUtils.addressToPublicKey(db, accountId);
		};

		server.get('/account/:accountId', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const sender = routeUtils.createSender(routeResultTypes.account);
			return db.accountsByIds([{ [type]: accountId }])
				.then(sender.sendOne(req.params.accountId, res, next));
		});

		// Get account blocks harvested and beneficiary by public key
		const accountBlocks = [
			{ dbField: 'block.signerPublicKey', routePostfix: '/harvest' },
			{ dbField: 'block.beneficiaryPublicKey', routePostfix: '/beneficiary' }
		];

		accountBlocks.forEach(blockType => {
			server.get(`/account/:accountId${blockType.routePostfix}`, (req, res, next) => {
				const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
				const pagingOptions = routeUtils.parsePagingArguments(req.params);
				const ordering = routeUtils.parseArgument(req.params, 'ordering', input => ('id' === input ? 1 : -1));
				const sender = routeUtils.createSender(routeResultTypes.blockWithId);

				return accountIdToPublicKey(type, accountId).then(accountPublicKey =>
					db.getBlocksBy(blockType.dbField, Buffer.from(accountPublicKey), pagingOptions.id, pagingOptions.pageSize, ordering)
						.then(sender.sendArray('accountId', res, next)))
					.catch(() => {
						sender.sendArray('accountId', res, next)([]);
					});
			});
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

		// region account transactions

		const transactionStates = [
			{ dbPostfix: 'Confirmed', routePostfix: '' },
			{ dbPostfix: 'Incoming', routePostfix: '/incoming' },
			{ dbPostfix: 'Unconfirmed', routePostfix: '/unconfirmed' }
		];

		const parseUintArrayString = arrayString =>
			routeUtils.parseArgumentAsArray({ param: arrayString.split(',') }, 'param', 'uint');

		transactionStates.concat(services.config.transactionStates).forEach(state => {
			server.get(`/account/:accountId/transactions${state.routePostfix}`, (req, res, next) => {
				const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
				const transactionTypes = req.params.type ? parseUintArrayString(req.params.type) : undefined;
				const pagingOptions = routeUtils.parsePagingArguments(req.params);
				const ordering = routeUtils.parseArgument(req.params, 'ordering', input => ('id' === input ? 1 : -1));

				const accountAddress = (AccountType.publicKey === type)
					? address.publicKeyToAddress(accountId, networkInfo.networks[services.config.network.name].id)
					: accountId;

				return db[`accountTransactions${state.dbPostfix}`](
					accountAddress,
					transactionTypes,
					pagingOptions.id,
					pagingOptions.pageSize,
					ordering
				).then(transactionSender.sendArray('accountId', res, next));
			});
		});

		server.get('/account/:accountId/transactions/outgoing', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const transactionTypes = req.params.type ? parseUintArrayString(req.params.type) : undefined;
			const pagingOptions = routeUtils.parsePagingArguments(req.params);
			const ordering = routeUtils.parseArgument(req.params, 'ordering', input => ('id' === input ? 1 : -1));

			return accountIdToPublicKey(type, accountId).then(publicKey =>
				db.accountTransactionsOutgoing(publicKey, transactionTypes, pagingOptions.id, pagingOptions.pageSize, ordering)
					.then(transactionSender.sendArray('accountId', res, next)))
				.catch(() => {
					transactionSender.sendArray('accountId', res, next)([]);
				});
		});

		// endregion
	}
};
