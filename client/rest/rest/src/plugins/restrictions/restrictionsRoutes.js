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
const merkleUtils = require('../../routes/merkleUtils');
const routeResultTypes = require('../../routes/routeResultTypes');
const routeUtils = require('../../routes/routeUtils');
const catapult = require('catapult-sdk');

const { PacketType } = catapult.packet;

const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db, services) => {
		const accountRestrictionsSender = routeUtils.createSender('accountRestrictions');

		// SEARCH
		server.get('/restrictions/account', (req, res, next) => {
			const { params } = req;
			const address = params.address ? routeUtils.parseArgument(params, 'address', 'address') : undefined;
			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });
			return db.accountRestrictions(address, options)
				.then(result => accountRestrictionsSender.sendPage(res, next)(result));
		});

		// GET ONE/MANY
		routeUtils.addGetPostDocumentRoutes(
			server,
			accountRestrictionsSender,
			{ base: '/restrictions/account', singular: 'address', plural: 'addresses' },
			params => db.accountRestrictionsByAddresses(params),
			routeUtils.namedParserMap.address
		);

		// MERKLE
		server.get('/restrictions/account/:address/merkle', (req, res, next) => {
			const encodedAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			const state = PacketType.accountRestrictionsStatePath;
			return merkleUtils.requestTree(services, state,
				encodedAddress).then(response => {
				res.send(response);
				next();
			});
		});

		// SEARCH
		const mosaicRestrictionSender = routeUtils.createSender(routeResultTypes.mosaicRestrictions);
		server.get('/restrictions/mosaic', (req, res, next) => {
			const { params } = req;
			const mosaicId = params.mosaicId ? routeUtils.parseArgument(params, 'mosaicId', uint64.fromHex) : undefined;
			const entryType = params.entryType ? routeUtils.parseArgument(params, 'entryType', 'uint') : undefined;
			const targetAddress = params.targetAddress ? routeUtils.parseArgument(params, 'targetAddress', 'address') : undefined;

			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });

			return db.mosaicRestrictions(mosaicId, entryType, targetAddress, options)
				.then(result => mosaicRestrictionSender.sendPage(res, next)(result));
		});

		// GET ONE MANY
		routeUtils.addGetPostDocumentRoutes(
			server,
			mosaicRestrictionSender,
			{ base: '/restrictions/mosaic', singular: 'compositeHash', plural: 'compositeHashes' },
			params => db.mosaicRestrictionByCompositeHash(params),
			routeUtils.namedParserMap.hash256
		);

		// GET MERKLE
		server.get('/restrictions/mosaic/:compositeHash/merkle', (req, res, next) => {
			const compositeHash = routeUtils.parseArgument(req.params, 'compositeHash', 'hash256');
			const state = PacketType.mosaicRestrictionsStatePath;
			return merkleUtils.requestTree(services, state,
				compositeHash).then(response => {
				res.send(response);
				next();
			});
		});
	}
};
