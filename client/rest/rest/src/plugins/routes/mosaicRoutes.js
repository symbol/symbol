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

const catapult = require('catapult-sdk');
const routeUtils = require('../../routes/routeUtils');

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

		server.get('/namespace/:namespaceId/mosaics', (req, res, next) => {
			const namespaceId = routeUtils.parseArgument(req.params, 'namespaceId', uint64.fromHex);
			const pagingOptions = routeUtils.parsePagingArguments(req.params);

			return db.mosaicsByNamespaceId(namespaceId, pagingOptions.id, pagingOptions.pageSize)
				.then(mosaicSender.sendArray('namespaceId', res, next));
		});

		server.post('/mosaic/names', (req, res, next) => {
			const mosaicIds = routeUtils.parseArgumentAsArray(req.params, 'mosaicIds', uint64.fromHex);
			const type = catapult.model.EntityType.mosaicDefinition;
			return db.catapultDb
				.findNamesByIds(mosaicIds, type, { id: 'mosaicId', name: 'name', parentId: 'parentId' })
				.then(routeUtils.createSender('mosaicNameTuple').sendArray('mosaicIds', res, next));
		});
	}
};
