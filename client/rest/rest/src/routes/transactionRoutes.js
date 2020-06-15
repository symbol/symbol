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
const errors = require('../server/errors');
const catapult = require('catapult-sdk');

const { convert } = catapult.utils;
const { PacketType } = catapult.packet;

const constants = {
	sizes: {
		hash: 64,
		objectId: 24
	}
};

const parseHeight = params => routeUtils.parseArgument(params, 'height', 'uint');

const parseObjectId = str => {
	if (!convert.isHexString(str))
		throw Error('must be 12-byte hex string');

	return str;
};

module.exports = {
	register: (server, db, services) => {
		const sender = routeUtils.createSender(routeResultTypes.transaction);

		routeUtils.addPutPacketRoute(
			server,
			services.connections,
			{ routeName: '/transactions', packetType: PacketType.pushTransactions },
			params => routeUtils.parseArgument(params, 'payload', convert.hexToUint8)
		);

		routeUtils.addGetPostDocumentRoutes(
			server,
			sender,
			{ base: '/transactions', singular: 'transactionId', plural: 'transactionIds' },
			// params has already been converted by a parser below, so it is: string - in case of objectId, Uint8Array - in case of hash
			params => (('string' === typeof params[0]) ? db.transactionsByIds(params) : db.transactionsByHashes(params)),
			(transactionId, index, array) => {
				if (0 < index && array[0].length !== transactionId.length)
					throw Error(`all ids must be homogeneous, element ${index}`);

				if (constants.sizes.objectId === transactionId.length)
					return parseObjectId(transactionId);
				if (constants.sizes.hash === transactionId.length)
					return convert.hexToUint8(transactionId);

				throw Error(`invalid length of transaction id '${transactionId}'`);
			}
		);

		server.get('/transactions', (req, res, next) => {
			const { params } = req;

			if (params.address && (params.signerPublicKey || params.recipientAddress)) {
				throw errors.createInvalidArgumentError(
					'can\'t filter by address if signerPublicKey or recipientAddress are already provided'
				);
			}

			if (params.group && !['confirmed', 'unconfirmed', 'partial'].includes(params.group))
				throw errors.createInvalidArgumentError('invalid transaction group provided');

			const filters = {
				height: params.height ? parseHeight(params) : undefined,
				address: params.address ? routeUtils.parseArgument(params, 'address', 'address') : undefined,
				signerPublicKey: params.signerPublicKey ? routeUtils.parseArgument(params, 'signerPublicKey', 'publicKey') : undefined,
				recipientAddress: params.recipientAddress ? routeUtils.parseArgument(params, 'recipientAddress', 'address') : undefined,
				transactionTypes: params.type ? routeUtils.parseArgumentAsArray(params, 'type', 'uint') : undefined,
				embedded: params.embedded ? routeUtils.parseArgument(params, 'embedded', 'boolean') : undefined,
				group: params.group
			};

			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });

			return db.transactions(filters, options)
				.then(result => routeUtils.createSender(routeResultTypes.transaction).sendPage(res, next)(result));
		});
	}
};
