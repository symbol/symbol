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
import catapult from '../../catapult-sdk/index.js';
import merkleUtils from '../../routes/merkleUtils.js';
import routeResultTypes from '../../routes/routeResultTypes.js';
import routeUtils from '../../routes/routeUtils.js';

const { PacketType } = catapult.packet;

export default {
	register: (server, db, services) => {
		const accountRestrictionsSender = routeUtils.createSender('accountRestrictions');

		// SEARCH
		server.get('/restrictions/account', async (request, reply) => {
			const { params } = request;
			const address = params.address ? routeUtils.parseArgument(params, 'address', 'address') : undefined;
			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });
			const result = await db.accountRestrictions(address, options);
			return reply.send(accountRestrictionsSender.sendPage()(result));
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
		server.get('/restrictions/account/:address/merkle', async (request, reply) => {
			const encodedAddress = routeUtils.parseArgument(request.params, 'address', 'address');
			const state = PacketType.accountRestrictionsStatePath;
			const response = await merkleUtils.requestTree(services, state, encodedAddress);
			return reply.send(response);
		});

		// SEARCH
		const mosaicRestrictionSender = routeUtils.createSender(routeResultTypes.mosaicRestrictions);
		server.get('/restrictions/mosaic', async (request, reply) => {
			const { params } = request;
			const mosaicId = params.mosaicId ? routeUtils.parseArgument(params, 'mosaicId', 'uint64hex') : undefined;
			const entryType = params.entryType ? routeUtils.parseArgument(params, 'entryType', 'uint') : undefined;
			const targetAddress = params.targetAddress ? routeUtils.parseArgument(params, 'targetAddress', 'address') : undefined;

			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, { id: 'objectId' });

			const result = await db.mosaicRestrictions(mosaicId, entryType, targetAddress, options);
			return reply.send(mosaicRestrictionSender.sendPage()(result));
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
		server.get('/restrictions/mosaic/:compositeHash/merkle', async (request, reply) => {
			const compositeHash = routeUtils.parseArgument(request.params, 'compositeHash', 'hash256');
			const state = PacketType.mosaicRestrictionsStatePath;
			const response = await merkleUtils.requestTree(services, state, compositeHash);
			return reply.send(response);
		});
	}
};
