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

const catapult = require('../../catapult-sdk/index');
const merkleUtils = require('../../routes/merkleUtils');
const routeResultTypes = require('../../routes/routeResultTypes');
const routeUtils = require('../../routes/routeUtils');

const { PacketType } = catapult.packet;

module.exports = {
	register: (server, db, services) => {
		const metadataSender = routeUtils.createSender(routeResultTypes.metadata);

		server.get('/metadata', (req, res, next) => {
			const { params } = req;
			const sourceAddress = params.sourceAddress ? routeUtils.parseArgument(params, 'sourceAddress', 'address') : undefined;
			const targetAddress = params.targetAddress ? routeUtils.parseArgument(params, 'targetAddress', 'address') : undefined;
			const scopedMetadataKey = params.scopedMetadataKey
				? routeUtils.parseArgument(params, 'scopedMetadataKey', 'uint64hex') : undefined;
			const targetId = params.targetId ? routeUtils.parseArgument(params, 'targetId', 'uint64hex') : undefined;
			const metadataType = params.metadataType ? routeUtils.parseArgument(params, 'metadataType', 'uint') : undefined;

			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });

			return db.metadata(sourceAddress, targetAddress, scopedMetadataKey, targetId, metadataType, options)
				.then(result => metadataSender.sendPage(res, next)(result));
		});

		routeUtils.addGetPostDocumentRoutes(
			server,
			metadataSender,
			{ base: '/metadata', singular: 'compositeHash', plural: 'compositeHashes' },
			params => db.metadatasByCompositeHash(params),
			routeUtils.namedParserMap.hash256
		);

		// this endpoint is here because it is expected to support requests by block other than <current block>
		server.get('/metadata/:compositeHash/merkle', (req, res, next) => {
			const compositeHash = routeUtils.parseArgument(req.params, 'compositeHash', 'hash256');
			const state = PacketType.metadataStatePath;
			return merkleUtils.requestTree(services, state, compositeHash).then(response => {
				res.send(response);
				next();
			});
		});
	}
};
