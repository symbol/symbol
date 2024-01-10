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

const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db, services) => {
		server.get('/blocks', (req, res, next) => {
			const { params } = req;

			const signerPublicKey = params.signerPublicKey ? routeUtils.parseArgument(params, 'signerPublicKey', 'publicKey') : undefined;
			const beneficiaryAddress = params.beneficiaryAddress
				? routeUtils.parseArgument(params, 'beneficiaryAddress', 'address')
				: undefined;

			const fromTimestamp = params.fromTimestamp ? routeUtils.parseArgument(params, 'fromTimestamp', 'uint64') : undefined;
			const toTimestamp = params.toTimestamp ? routeUtils.parseArgument(params, 'toTimestamp', 'uint64') : undefined;

			const offsetParsers = {
				id: 'objectId',
				height: 'uint64'
			};
			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, offsetParsers);

			return db.blocks(signerPublicKey, beneficiaryAddress, fromTimestamp, toTimestamp, options)
				.then(result => routeUtils.createSender(routeResultTypes.block).sendPage(res, next)(result));
		});

		server.get('/blocks/:height', (req, res, next) => {
			const height = routeUtils.parseArgument(req.params, 'height', 'uint64');

			return dbFacade.runHeightDependentOperation(db, height, () => db.blockAtHeight(height))
				.then(result => result.payload)
				.then(routeUtils.createSender(routeResultTypes.block).sendOne(uint64.toString(height), res, next));
		});

		server.get(
			'/blocks/:height/transactions/:hash/merkle',
			routeUtils.blockRouteMerkleProcessor(db, 'transactionsCount', 'transactionMerkleTree')
		);
	}
};
