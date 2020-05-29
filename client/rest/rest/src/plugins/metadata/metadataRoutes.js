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

const routeUtils = require('../../routes/routeUtils');
const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');

const { Long } = MongoDb;
const { metadata } = catapult.model;
const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db) => {
		const idFilter = id => ({ 'metadataEntry.targetId': new Long(id[0], id[1]) });

		const addMetadataEndpointsFor = entity => {
			server.get(`/metadata/${entity}/:${entity}Id`, (req, res, next) => {
				const entityId = routeUtils.parseArgument(req.params, `${entity}Id`, uint64.fromHex);
				const pagingOptions = routeUtils.parsePagingArguments(req.params);
				const ordering = routeUtils.parseArgument(req.params, 'ordering', input => ('id' === input ? 1 : -1));

				return db.getMetadataWithPagination(
					metadata.metadataType[entity],
					idFilter(entityId),
					pagingOptions.id,
					pagingOptions.pageSize,
					ordering
				).then(metadataEntries => routeUtils.createSender('metadata').sendOne(entityId, res, next)({ metadataEntries }));
			});

			server.get(`/metadata/${entity}/:${entity}Id/key/:key`, (req, res, next) => {
				const entityId = routeUtils.parseArgument(req.params, `${entity}Id`, uint64.fromHex);
				const scopedMetadataKey = routeUtils.parseArgument(req.params, 'key', uint64.fromHex);

				return db.getMetadataByKey(metadata.metadataType[entity], idFilter(entityId), scopedMetadataKey)
					.then(metadataEntries =>
						routeUtils.createSender('metadata').sendOne(scopedMetadataKey, res, next)({ metadataEntries }));
			});

			server.get(`/metadata/${entity}/:${entity}Id/key/:key/sender/:senderAddress`, (req, res, next) => {
				const entityId = routeUtils.parseArgument(req.params, `${entity}Id`, uint64.fromHex);
				const scopedMetadataKey = routeUtils.parseArgument(req.params, 'key', uint64.fromHex);
				const sourceAddress = routeUtils.parseArgument(req.params, 'senderAddress', 'address');

				return db.getMetadataByKeyAndSender(metadata.metadataType[entity], idFilter(entityId), scopedMetadataKey, sourceAddress)
					.then(metadataResult => routeUtils.createSender('metadata.entry').sendOne(sourceAddress, res, next)(metadataResult));
			});
		};

		// region account metadata

		server.get('/metadata/account/:address', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			const pagingOptions = routeUtils.parsePagingArguments(req.params);
			const ordering = routeUtils.parseArgument(req.params, 'ordering', input => ('id' === input ? 1 : -1));

			return db.getMetadataWithPagination(
				metadata.metadataType.account,
				{ 'metadataEntry.targetAddress': Buffer.from(accountAddress) },
				pagingOptions.id,
				pagingOptions.pageSize,
				ordering
			).then(
				metadataEntries => routeUtils.createSender('metadata').sendOne('address', res, next)({ metadataEntries })
			);
		});

		server.get('/metadata/account/:address/key/:key', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			const scopedMetadataKey = routeUtils.parseArgument(req.params, 'key', uint64.fromHex);

			return db.getMetadataByKey(
				metadata.metadataType.account,
				{ 'metadataEntry.targetAddress': Buffer.from(accountAddress) },
				scopedMetadataKey
			).then(
				metadataEntries => routeUtils.createSender('metadata').sendOne(scopedMetadataKey, res, next)({ metadataEntries })
			);
		});

		server.get('/metadata/account/:address/key/:key/sender/:senderAddress', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			const scopedMetadataKey = routeUtils.parseArgument(req.params, 'key', uint64.fromHex);
			const sourceAddress = routeUtils.parseArgument(req.params, 'senderAddress', 'address');

			return db.getMetadataByKeyAndSender(
				metadata.metadataType.account,
				{ 'metadataEntry.targetAddress': Buffer.from(accountAddress) },
				scopedMetadataKey,
				sourceAddress
			).then(
				metadataResult => routeUtils.createSender('metadata.entry').sendOne(sourceAddress, res, next)(metadataResult)
			);
		});

		// endregion

		// mosaic metadata
		addMetadataEndpointsFor('mosaic');

		// namespace metadata
		addMetadataEndpointsFor('namespace');
	}
};
