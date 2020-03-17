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
const errors = require('../../server/errors');
const AccountType = require('../AccountType');
const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');

const { Long } = MongoDb;
const { metadata } = catapult.model;
const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db) => {
		const accountIdToPublicKey = (type, accountId) => {
			if (AccountType.publicKey === type)
				return Promise.resolve(accountId);

			return routeUtils.addressToPublicKey(db.catapultDb, accountId);
		};

		const accountFilter = publicKey => ({ 'metadataEntry.targetPublicKey': Buffer.from(publicKey) });

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

			server.get(`/metadata/${entity}/:${entity}Id/key/:key/sender/:publicKey`, (req, res, next) => {
				const entityId = routeUtils.parseArgument(req.params, `${entity}Id`, uint64.fromHex);
				const scopedMetadataKey = routeUtils.parseArgument(req.params, 'key', uint64.fromHex);
				const senderPublicKey = routeUtils.parseArgument(req.params, 'publicKey', 'publicKey');

				return db.getMetadataByKeyAndSender(metadata.metadataType[entity], idFilter(entityId), scopedMetadataKey, senderPublicKey)
					.then(metadataResult => routeUtils.createSender('metadata.entry').sendOne(senderPublicKey, res, next)(metadataResult));
			});
		};

		// region account metadata

		server.get('/metadata/account/:accountId', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const pagingOptions = routeUtils.parsePagingArguments(req.params);
			const ordering = routeUtils.parseArgument(req.params, 'ordering', input => ('id' === input ? 1 : -1));

			return accountIdToPublicKey(type, accountId)
				.then(publicKey =>
					db.getMetadataWithPagination(
						metadata.metadataType.account,
						accountFilter(publicKey),
						pagingOptions.id,
						pagingOptions.pageSize,
						ordering
					).then(metadataEntries => routeUtils.createSender('metadata').sendOne(accountId, res, next)({ metadataEntries })))
				.catch(() => {
					res.send(errors.createNotFoundError(req.params.accountId));
					return next();
				});
		});

		server.get('/metadata/account/:accountId/key/:key', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const scopedMetadataKey = routeUtils.parseArgument(req.params, 'key', uint64.fromHex);

			return accountIdToPublicKey(type, accountId)
				.then(publicKey => db.getMetadataByKey(metadata.metadataType.account, accountFilter(publicKey), scopedMetadataKey)
					.then(metadataEntries =>
						routeUtils.createSender('metadata').sendOne(scopedMetadataKey, res, next)({ metadataEntries })))
				.catch(() => {
					res.send(errors.createNotFoundError(req.params.accountId));
					return next();
				});
		});

		server.get('/metadata/account/:accountId/key/:key/sender/:publicKey', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const scopedMetadataKey = routeUtils.parseArgument(req.params, 'key', uint64.fromHex);
			const senderPublicKey = routeUtils.parseArgument(req.params, 'publicKey', 'publicKey');

			return accountIdToPublicKey(type, accountId)
				.then(publicKey =>
					db.getMetadataByKeyAndSender(
						metadata.metadataType.account,
						accountFilter(publicKey),
						scopedMetadataKey,
						senderPublicKey
					).then(metadataResult => routeUtils.createSender('metadata.entry').sendOne(senderPublicKey, res, next)(metadataResult)))
				.catch(() => {
					res.send(errors.createNotFoundError(req.params.accountId));
					return next();
				});
		});

		// endregion

		// mosaic metadata
		addMetadataEndpointsFor('mosaic');

		// namespace metadata
		addMetadataEndpointsFor('namespace');
	}
};
