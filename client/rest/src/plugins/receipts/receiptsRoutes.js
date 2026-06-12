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

import routeResultTypes from '../../routes/routeResultTypes.js';
import routeUtils from '../../routes/routeUtils.js';
import errors from '../../server/errors.js';

export default {
	register: (server, db, services) => {
		server.get('/statements/transaction', async (request, reply) => {
			const { params } = request;
			const filters = {
				height: params.height ? routeUtils.parseArgument(params, 'height', 'uint64') : undefined,
				fromHeight: params.fromHeight ? routeUtils.parseArgument(params, 'fromHeight', 'uint64') : undefined,
				toHeight: params.toHeight ? routeUtils.parseArgument(params, 'toHeight', 'uint64') : undefined,
				receiptType: params.receiptType ? routeUtils.parseArgumentAsArray(params, 'receiptType', 'uint') : undefined,
				recipientAddress: params.recipientAddress ? routeUtils.parseArgument(params, 'recipientAddress', 'address') : undefined,
				senderAddress: params.senderAddress ? routeUtils.parseArgument(params, 'senderAddress', 'address') : undefined,
				targetAddress: params.targetAddress ? routeUtils.parseArgument(params, 'targetAddress', 'address') : undefined,
				artifactId: params.artifactId ? routeUtils.parseArgument(params, 'artifactId', 'uint64hex') : undefined
			};

			const options = routeUtils.parsePaginationArguments(request.params, services.config.pageSize, { id: 'objectId' });

			const result = await db.transactionStatements(filters, options);
			return reply.send(routeUtils.createSender(routeResultTypes.transactionStatement).sendPage()(result));
		});

		server.get('/statements/resolutions/:artifact', async (request, reply) => {
			const { artifact } = request.params;
			if (!artifact || !['address', 'mosaic'].includes(artifact))
				throw errors.createNotFoundError();

			const height = request.params.height ? routeUtils.parseArgument(request.params, 'height', 'uint64') : undefined;
			const options = routeUtils.parsePaginationArguments(request.params, services.config.pageSize, { id: 'objectId' });

			const result = await db.artifactStatements(height, artifact, options);
			return reply.send(routeUtils.createSender(routeResultTypes[`${artifact}ResolutionStatement`]).sendPage()(result));
		});

		server.get(
			'/blocks/:height/statements/:hash/merkle',
			routeUtils.blockRouteMerkleProcessor(db.catapultDb, 'statementsCount', 'statementMerkleTree')
		);
	}
};
