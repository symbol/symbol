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

module.exports = {
	register: (server, db) => {
		server.get('/diagnostic/storage', (req, res, next) =>
			db.storageInfo().then(storageInfo => {
				res.send({ payload: storageInfo, type: routeResultTypes.storageInfo });
				next();
			}));

		server.get('/diagnostic/blocks/:height/limit/:limit', (req, res, next) => {
			const parseUint = paramName => routeUtils.parseArgument(req.params, paramName, 'uint');
			const height = parseUint('height');
			const count = parseUint('limit');

			return db.blocksFrom(height, count).then(blocks => {
				res.send({ payload: blocks, type: routeResultTypes.block });
				next();
			});
		});
	}
};
