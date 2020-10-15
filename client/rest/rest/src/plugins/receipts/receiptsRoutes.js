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
const { NotFoundError } = require('restify-errors');

module.exports = {
	register: (server, db, services) => {
		server.get('/statements/transaction', (req, res, next) => {
			const { params } = req;
			const filters = {
				height: params.height ? routeUtils.parseArgument(params, 'height', 'uint64') : undefined,
				receiptType: params.receiptType ? routeUtils.parseArgumentAsArray(params, 'receiptType', 'uint') : undefined,
				recipientAddress: params.recipientAddress ? routeUtils.parseArgument(params, 'recipientAddress', 'address') : undefined,
				senderAddress: params.senderAddress ? routeUtils.parseArgument(params, 'senderAddress', 'address') : undefined,
				targetAddress: params.targetAddress ? routeUtils.parseArgument(params, 'targetAddress', 'address') : undefined,
				artifactId: params.artifactId ? routeUtils.parseArgument(params, 'artifactId', 'uint64hex') : undefined
			};

			const options = routeUtils.parsePaginationArguments(req.params, services.config.pageSize, { id: 'objectId' });

			return db.transactionStatements(filters, options)
				.then(result => routeUtils.createSender(routeResultTypes.transactionStatement).sendPage(res, next)(result));
		});

		server.get('/statements/resolutions/:artifact', (req, res, next) => {
			const { artifact } = req.params;
			if (!artifact || !['address', 'mosaic'].includes(artifact))
				return next(new NotFoundError());

			const height = req.params.height ? routeUtils.parseArgument(req.params, 'height', 'uint64') : undefined;
			const options = routeUtils.parsePaginationArguments(req.params, services.config.pageSize, { id: 'objectId' });

			return db.artifactStatements(height, artifact, options)
				.then(result => routeUtils.createSender(routeResultTypes[`${artifact}ResolutionStatement`]).sendPage(res, next)(result));
		});

		server.get(
			'/blocks/:height/statements/:hash/merkle',
			routeUtils.blockRouteMerkleProcessor(db.catapultDb, 'statementsCount', 'statementMerkleTree')
		);
	}
};
