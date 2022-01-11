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

const dbFacade = require('./dbFacade');
const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');
const catapult = require('../catapult-sdk/index');

const { convert } = catapult.utils;
const { constants } = catapult;

module.exports = {
	register: (server, db, services) => {
		routeUtils.addGetPostDocumentRoutes(
			server,
			routeUtils.createSender(routeResultTypes.transactionStatus),
			{ base: '/transactionStatus', singular: 'hash', plural: 'hashes' },
			params => dbFacade.transactionStatusesByHashes(db, params, services.config.transactionStates),
			hash => {
				if (2 * constants.sizes.hash256 === hash.length)
					return convert.hexToUint8(hash);

				throw Error(`invalid length of hash '${hash.length}'`);
			}
		);
	}
};
