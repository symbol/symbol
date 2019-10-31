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
const errors = require('../../server/errors');
const AccountType = require('../AccountType');
const catapult = require('catapult-sdk');

const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db) => {
		const mosaicSender = routeUtils.createSender('mosaicDescriptor');

		routeUtils.addGetPostDocumentRoutes(
			server,
			mosaicSender,
			{ base: '/mosaic', singular: 'mosaicId', plural: 'mosaicIds' },
			params => db.mosaicsByIds(params),
			uint64.fromHex
		);

		const ownedMosaicsSender = routeUtils.createSender('ownedMosaics');

		server.get('/account/:accountId/mosaics', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');

			return db.mosaicsByOwners(type, [accountId])
				.then(mosaics => ownedMosaicsSender.sendOne('accountId', res, next)({ mosaics }));
		});

		server.post('/account/mosaics', (req, res, next) => {
			if (req.params.publicKeys && req.params.addresses)
				throw errors.createInvalidArgumentError('publicKeys and addresses cannot both be provided');

			const idOptions = Array.isArray(req.params.publicKeys)
				? { keyName: 'publicKeys', parserName: 'publicKey', type: AccountType.publicKey }
				: { keyName: 'addresses', parserName: 'address', type: AccountType.address };

			const accountIds = routeUtils.parseArgumentAsArray(req.params, idOptions.keyName, idOptions.parserName);
			return db.mosaicsByOwners(idOptions.type, accountIds)
				.then(mosaics => ownedMosaicsSender.sendOne(idOptions.keyName, res, next)({ mosaics }));
		});
	}
};
