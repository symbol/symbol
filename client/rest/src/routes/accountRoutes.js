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

import merkleUtils from './merkleUtils.js';
import routeResultTypes from './routeResultTypes.js';
import routeUtils from './routeUtils.js';
import catapult from '../catapult-sdk/index.js';
import AccountType from '../plugins/AccountType.js';
import errors from '../server/errors.js';
import { NetworkLocator, PublicKey } from 'symbol-sdk';
import { Network } from 'symbol-sdk/symbol';

const { PacketType } = catapult.packet;

export default {
	register: (server, db, services) => {
		const sender = routeUtils.createSender(routeResultTypes.account);

		server.get('/accounts', async (request, reply) => {
			const address = request.params.address ? routeUtils.parseArgument(request.params, 'address', 'address') : undefined;
			const mosaicId = request.params.mosaicId ? routeUtils.parseArgument(request.params, 'mosaicId', 'uint64hex') : undefined;

			const offsetParsers = {
				id: 'objectId',
				balance: 'uint64'
			};
			const options = routeUtils.parsePaginationArguments(request.params, services.config.pageSize, offsetParsers);

			if ('balance' === options.sortField && !mosaicId)
				throw errors.createInvalidArgumentError('mosaicId must be provided when sorting by balance');

			const result = await db.accounts(address, mosaicId, options);
			return reply.send(sender.sendPage()(result));
		});

		server.get('/accounts/:accountId', async (request, reply) => {
			const [type, accountId] = routeUtils.parseArgument(request.params, 'accountId', 'accountId');
			const result = await db.accountsByIds([{ [type]: accountId }]);
			return reply.send(sender.sendOne(request.params.accountId)(result));
		});

		server.post('/accounts', async (request, reply) => {
			if (request.params.publicKeys && request.params.addresses)
				throw errors.createInvalidArgumentError('publicKeys and addresses cannot both be provided');

			const idOptions = Array.isArray(request.params.publicKeys)
				? { keyName: 'publicKeys', parserName: 'publicKey', type: AccountType.publicKey }
				: { keyName: 'addresses', parserName: 'address', type: AccountType.address };

			const accountIds = routeUtils.parseArgumentAsArray(request.params, idOptions.keyName, idOptions.parserName);

			const result = await db.accountsByIds(accountIds.map(accountId => ({ [idOptions.type]: accountId })));
			return reply.send(sender.sendArray(idOptions.keyName)(result));
		});

		// this endpoint is here because it is expected to support requests by block other than <current block>
		server.get('/accounts/:accountId/merkle', async (request, reply) => {
			const [type, accountId] = routeUtils.parseArgument(request.params, 'accountId', 'accountId');
			const encodedAddress = 'publicKey' === type
				? NetworkLocator.findByIdentifier(Network.NETWORKS, db.networkId).publicKeyToAddress(new PublicKey(accountId)).bytes
				: accountId;
			const state = PacketType.accountStatePath;
			const response = await merkleUtils.requestTree(services, state, encodedAddress);
			return reply.send(response);
		});
	}
};
