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

const dbFacade = require('./dbFacade');
const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');
const errors = require('../server/errors');
const stateTreesCodec = require('../sockets/stateTreesCodec');
const catapult = require('catapult-sdk');

const { constants } = catapult;
const { uint64 } = catapult.utils;
const packetHeader = catapult.packet.header;
const { StatePathPacketTypes } = catapult.packet;
const { BinaryParser } = catapult.parser;

module.exports = {
	register: (server, db, services) => {
		server.get('/blocks', (req, res, next) => {
			const { params } = req;

			const signerPublicKey = params.signerPublicKey ? routeUtils.parseArgument(params, 'signerPublicKey', 'publicKey') : undefined;
			const beneficiaryAddress = params.beneficiaryAddress
				? routeUtils.parseArgument(params, 'beneficiaryAddress', 'address')
				: undefined;

			const offsetParsers = {
				id: 'objectId',
				height: 'uint64'
			};
			const options = routeUtils.parsePaginationArguments(params, services.config.pageSize, offsetParsers);

			return db.blocks(signerPublicKey, beneficiaryAddress, options)
				.then(result => routeUtils.createSender(routeResultTypes.block).sendPage(res, next)(result));
		});

		server.get('/blocks/:height', (req, res, next) => {
			const height = routeUtils.parseArgument(req.params, 'height', 'uint64');

			return dbFacade.runHeightDependentOperation(db, height, () => db.blockAtHeight(height))
				.then(result => result.payload)
				.then(routeUtils.createSender(routeResultTypes.block).sendOne(uint64.toString(height), res, next));
		});

		server.get(
			'/blocks/:height/transactions/:hash/merkle',
			routeUtils.blockRouteMerkleProcessor(db, 'transactionsCount', 'transactionMerkleTree')
		);

		const buildResponse = (packet, codec, resultType) => {
			const binaryParser = new BinaryParser();
			binaryParser.push(packet.payload);
			return { payload: codec.deserialize(binaryParser), type: resultType, formatter: 'ws' };
		};

		// this endpoint is here because it is expected to support requests by block other than <current block>
		server.get('/state/:state/hash/:hash/merkle', (req, res, next) => {
			const { state } = req.params;
			const hash = routeUtils.parseArgument(req.params, 'hash', 'hash256');

			if (!StatePathPacketTypes.includes(state))
				throw errors.createInvalidArgumentError('invalid `state` provided');

			const { connections } = services;
			const { timeout } = services.config.apiNode;

			const headerBuffer = packetHeader.createBuffer(state, packetHeader.size + constants.sizes.hash256);
			const packetBuffer = Buffer.concat([headerBuffer, hash]);
			return connections.singleUse()
				.then(connection => connection.pushPull(packetBuffer, timeout))
				.then(packet => {
					res.send(buildResponse(packet, stateTreesCodec, routeResultTypes.stateTree));
					next();
				});
		});
	}
};
