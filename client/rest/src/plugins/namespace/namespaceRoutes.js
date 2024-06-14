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

import namespaceUtils from './namespaceUtils.js';
import catapult from '../../catapult-sdk/index.js';
import { convertToLong } from '../../db/dbUtils.js';
import merkleUtils from '../../routes/merkleUtils.js';
import routeUtils from '../../routes/routeUtils.js';
import MongoDb from 'mongodb';
import { utils } from 'symbol-sdk';
import { models } from 'symbol-sdk/symbol';

const { PacketType } = catapult.packet;
const { Binary } = MongoDb;

export default {
	register: (server, db, services) => {
		const namespaceSender = routeUtils.createSender('namespaceDescriptor');

		server.get('/namespaces', (req, res, next) => {
			const { params } = req;

			const ownerAddress = params.ownerAddress ? routeUtils.parseArgument(params, 'ownerAddress', 'address') : undefined;
			const registrationType = params.registrationType ? routeUtils.parseArgument(params, 'registrationType', 'uint') : undefined;
			const level0 = params.level0 ? routeUtils.parseArgument(req.params, 'level0', routeUtils.namedParserMap.uint64hex) : undefined;
			const aliasType = params.aliasType ? routeUtils.parseArgument(params, 'aliasType', 'uint') : undefined;

			const options = routeUtils.parsePaginationArguments(req.params, services.config.pageSize, { id: 'objectId' });

			return db.namespaces(aliasType, level0, ownerAddress, registrationType, options)
				.then(result => namespaceSender.sendPage(res, next)(result));
		});

		server.get('/namespaces/:namespaceId', (req, res, next) => {
			const namespaceId = routeUtils.parseArgument(req.params, 'namespaceId', routeUtils.namedParserMap.uint64hex);
			return db.namespaceById(namespaceId)
				.then(namespaceSender.sendOne(req.params.namespaceId, res, next));
		});

		const collectNames = (namespaceNameTuples, namespaceIds) => {
			const type = models.TransactionType.NAMESPACE_REGISTRATION;
			return db.catapultDb.findNamesByIds(namespaceIds, type, { id: 'id', name: 'name', parentId: 'parentId' })
				.then(nameTuples => {
					nameTuples.forEach(nameTuple => {
						// db returns null instead of undefined when parentId is not present
						if (null === nameTuple.parentId)
							delete nameTuple.parentId;

						namespaceNameTuples.push(nameTuple);
					});

					// process all parent namespaces next
					return nameTuples
						.filter(nameTuple => undefined !== nameTuple.parentId)
						.map(nameTuple => nameTuple.parentId);
				});
		};

		server.post('/namespaces/names', (req, res, next) => {
			const namespaceIds = routeUtils.parseArgumentAsArray(req.params, 'namespaceIds', routeUtils.namedParserMap.uint64hex);
			const nameTuplesFuture = new Promise(resolve => {
				const namespaceNameTuples = [];
				const chain = nextIds => {
					if (0 === nextIds.length)
						resolve(namespaceNameTuples);
					else
						collectNames(namespaceNameTuples, nextIds).then(chain);
				};

				collectNames(namespaceNameTuples, namespaceIds).then(chain);
			});

			return nameTuplesFuture.then(routeUtils.createSender('namespaceNameTuple').sendArray('namespaceIds', res, next));
		});

		server.post('/namespaces/mosaic/names', namespaceUtils.aliasNamesRoutesProcessor(
			db,
			catapult.model.NamespaceAliasType.MOSAIC_ID,
			req => routeUtils.parseArgumentAsArray(req.params, 'mosaicIds', routeUtils.namedParserMap.uint64hex).map(convertToLong),
			(namespace, id) => namespace.namespace.alias.mosaicId.equals(id),
			'mosaicId',
			'mosaicNames'
		));

		server.post('/namespaces/account/names', namespaceUtils.aliasNamesRoutesProcessor(
			db,
			catapult.model.NamespaceAliasType.ADDRESS,
			req => routeUtils.parseArgumentAsArray(req.params, 'addresses', 'address'),
			(namespace, id) => Buffer.from(namespace.namespace.alias.address.value())
				.equals(Buffer.from(new Binary(Buffer.from(id)).value())),
			'address',
			'accountNames'
		));

		// this endpoint is here because it is expected to support requests by block other than <current block>
		server.get('/namespaces/:namespaceId/merkle', (req, res, next) => {
			const namespaceId = routeUtils.parseArgument(req.params, 'namespaceId', 'uint64hex');
			const state = PacketType.namespaceStatePath;
			return merkleUtils.requestTree(services, state, utils.intToBytes(namespaceId, 8)).then(response => {
				res.send(response);
				next();
			});
		});
	}
};
