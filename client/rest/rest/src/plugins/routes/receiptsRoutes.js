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

const dbFacade = require('../../routes/dbFacade');
const errors = require('../../server/errors');
const routeResultTypes = require('../../routes/routeResultTypes');
const routeUtils = require('../../routes/routeUtils');

const parseHeight = params => routeUtils.parseArgument(params, 'height', 'uint');

const getStatementsPromise = (db, height, operation) => dbFacade.runHeightDependentOperation(db, height, () => operation(height));

module.exports = {
	register: (server, db) => {
		server.get('/block/:height/receipts', (req, res, next) => {
			const height = parseHeight(req.params);

			return Promise.all([
				getStatementsPromise(db, height, db.transactionStatementsAtHeight),
				getStatementsPromise(db, height, db.addressResolutionStatementsAtHeight),
				getStatementsPromise(db, height, db.mosaicResolutionStatementsAtHeight)
			]).then(results => {
				const [
					transactionStatementsInfo,
					addressResolutionStatementsInfo,
					mosaicResolutionStatementsInfo
				] = results;

				if (results.some(result => !result.isRequestValid)) {
					res.send(errors.createNotFoundError(height));
					return next();
				}

				const result = {
					transactionStatements: transactionStatementsInfo.payload,
					addressResolutionStatements: addressResolutionStatementsInfo.payload,
					mosaicResolutionStatements: mosaicResolutionStatementsInfo.payload
				};

				res.send({
					payload: result,
					type: routeResultTypes.receipts
				});

				return next();
			});
		});

		server.get(
			'/block/:height/receipt/:hash/merkle',
			routeUtils.blockRouteMerkleProcessor(db, 'numStatements', 'statementMerkleTree')
		);
	}
};
