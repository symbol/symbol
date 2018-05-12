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

const curryStripProperties = (properties, next) => chainInfo => {
	properties.forEach(property => {
		delete chainInfo[property];
	});

	next(chainInfo);
};

const currySend = (db, res, next) => chainInfo => {
	res.send({ payload: chainInfo, type: routeResultTypes.chainInfo });
	next();
};

module.exports = {
	register: (server, db) => {
		server.get('/chain/height', (req, res, next) =>
			db.chainInfo().then(curryStripProperties(['scoreLow', 'scoreHigh'], currySend(db, res, next))));

		server.get('/chain/score', (req, res, next) =>
			db.chainInfo().then(curryStripProperties(['height'], currySend(db, res, next))));
	}
};
