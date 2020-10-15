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
const errors = require('../server/errors');
const catapult = require('catapult-sdk');
const { NotFoundError, InvalidArgumentError } = require('restify-errors');

const { convert } = catapult.utils;
const { PacketType } = catapult.packet;

const constants = {
	sizes: {
		hash: 64,
		objectId: 24
	}
};

const isValidTransactionGroup = group => ['confirmed', 'unconfirmed', 'partial'].includes(group);

module.exports = {
	register: (server, db, services) => {
		const sender = routeUtils.createSender(routeResultTypes.transaction);

		routeUtils.addPutPacketRoute(
			server,
			services.connections,
			{ routeName: '/transactions', packetType: PacketType.pushTransactions },
			params => routeUtils.parseArgument(params, 'payload', convert.hexToUint8)
		);

		server.get('/transactions/:group', (req, res, next) => {
			const { params } = req;

			if (!isValidTransactionGroup(params.group))
				return next(new NotFoundError());

			if (params.address && (params.signerPublicKey || params.recipientAddress)) {
				return next(new InvalidArgumentError(
					'can\'t filter by address if signerPublicKey or recipientAddress are already provided'
				));
			}

			if ((params.fromTransferAmount || params.toTransferAmount) && !params.transferMosaicId)
				return next(new InvalidArgumentError('can\'t filter by transfer amount if `transferMosaicId` is not provided'));

			const filters = {
				height: params.height ? routeUtils.parseArgument(params, 'height', 'uint64') : undefined,
				fromHeight: params.fromHeight ? routeUtils.parseArgument(params, 'fromHeight', 'uint64') : undefined,
				toHeight: params.toHeight ? routeUtils.parseArgument(params, 'toHeight', 'uint64') : undefined,
				address: params.address ? routeUtils.parseArgument(params, 'address', 'address') : undefined,
				signerPublicKey: params.signerPublicKey ? routeUtils.parseArgument(params, 'signerPublicKey', 'publicKey') : undefined,
				recipientAddress: params.recipientAddress ? routeUtils.parseArgument(params, 'recipientAddress', 'address') : undefined,
				transactionTypes: params.type ? routeUtils.parseArgumentAsArray(params, 'type', 'uint') : undefined,
				embedded: params.embedded ? routeUtils.parseArgument(params, 'embedded', 'boolean') : undefined,

				/** transfer transaction specific filters */
				transferMosaicId: params.transferMosaicId ? routeUtils.parseArgument(params, 'transferMosaicId', 'uint64') : undefined,
				fromTransferAmount: params.fromTransferAmount
					? routeUtils.parseArgument(params, 'fromTransferAmount', 'uint64') : undefined,
				toTransferAmount: params.toTransferAmount ? routeUtils.parseArgument(params, 'toTransferAmount', 'uint64') : undefined
			};

			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });

			return db.transactions(params.group, filters, options)
				.then(result => routeUtils.createSender(routeResultTypes.transaction).sendPage(res, next)(result));
		});

		server.get('/transactions/:group/:transactionId', (req, res, next) => {
			const { params } = req;

			if (!isValidTransactionGroup(params.group))
				return next(new NotFoundError());

			let paramType = constants.sizes.objectId === params.transactionId.length ? 'id' : undefined;
			paramType = constants.sizes.hash === params.transactionId.length ? 'hash' : paramType;
			if (!paramType)
				throw Error(`invalid length of transaction id '${params.transactionId}'`);

			const transactionId = routeUtils.parseArgument(params, 'transactionId', 'id' === paramType ? 'objectId' : 'hash256');

			const dbTransactionsRetriever = 'id' === paramType ? 'transactionsByIds' : 'transactionsByHashes';
			return db[dbTransactionsRetriever](params.group, [transactionId]).then(sender.sendOne(params.transactionId, res, next));
		});

		server.post('/transactions/:group', (req, res, next) => {
			const { params } = req;

			if (!isValidTransactionGroup(params.group))
				return next(new NotFoundError());

			if ((req.params.transactionIds && req.params.hashes) || (!params.transactionIds && !params.hashes))
				throw errors.createInvalidArgumentError('either ids or hashes must be provided');

			// normalize ids arg to be either in the transcationIds object or hashes (this is expected to change in the near future)
			if (params.transactionIds && constants.sizes.hash === params.transactionIds[0].length) {
				params.hashes = params.transactionIds;
				delete params.transactionIds;
			}

			const transactionIds = params.transactionIds
				? routeUtils.parseArgumentAsArray(params, 'transactionIds', 'objectId')
				: routeUtils.parseArgumentAsArray(params, 'hashes', 'hash256');

			const dbTransactionsRetriever = params.transactionIds ? 'transactionsByIds' : 'transactionsByHashes';
			return db[dbTransactionsRetriever](params.group, transactionIds)
				.then(sender.sendArray(params.transactionIds || params.hashes, res, next));
		});
	}
};
