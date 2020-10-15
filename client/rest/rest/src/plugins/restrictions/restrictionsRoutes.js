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

const routeResultTypes = require('../../routes/routeResultTypes');
const routeUtils = require('../../routes/routeUtils');
const catapult = require('catapult-sdk');

const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db, services) => {
		const accountRestrictionsSender = routeUtils.createSender('accountRestrictions');

		server.get('/restrictions/account/:address', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			return db.accountRestrictionsByAddresses([accountAddress])
				.then(accountRestrictionsSender.sendOne(req.params.address, res, next));
		});

		server.get('/restrictions/mosaic', (req, res, next) => {
			const { params } = req;
			const mosaicId = params.mosaicId ? routeUtils.parseArgument(params, 'mosaicId', uint64.fromHex) : undefined;
			const entryType = params.entryType ? routeUtils.parseArgument(params, 'entryType', 'uint') : undefined;
			const targetAddress = params.targetAddress ? routeUtils.parseArgument(params, 'targetAddress', 'address') : undefined;

			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });

			return db.mosaicRestrictions(mosaicId, entryType, targetAddress, options)
				.then(result => routeUtils.createSender(routeResultTypes.mosaicRestrictions).sendPage(res, next)(result));
		});
	}
};
