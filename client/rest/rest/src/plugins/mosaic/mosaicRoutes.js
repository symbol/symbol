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
const routeUtils = require('../../routes/routeUtils');
const catapult = require('catapult-sdk');
const ini = require('ini');
const fs = require('fs');
const util = require('util');

const { PacketType } = catapult.packet;

const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db, services) => {
		const mosaicSender = routeUtils.createSender('mosaicDescriptor');

		server.get('/mosaics', (req, res, next) => {
			const ownerAddress = req.params.ownerAddress ? routeUtils.parseArgument(req.params, 'ownerAddress', 'address') : undefined;

			const options = routeUtils.parsePaginationArguments(req.params, services.config.pageSize, { id: 'objectId' });

			return db.mosaics(ownerAddress, options)
				.then(result => mosaicSender.sendPage(res, next)(result));
		});

		routeUtils.addGetPostDocumentRoutes(
			server,
			mosaicSender,
			{ base: '/mosaics', singular: 'mosaicId', plural: 'mosaicIds' },
			params => db.mosaicsByIds(params),
			uint64.fromHex
		);

		// this endpoint is here because it is expected to support requests by block other than <current block>
		server.get('/mosaics/:mosaicId/merkle', (req, res, next) => {
			const mosaicId = routeUtils.parseArgument(req.params, 'mosaicId',
				'uint64hex');
			const state = PacketType.mosaicStatePath;

			return merkleUtils.requestTree(services, state,
				uint64.toBytes(mosaicId)).then(response => {
				res.send(response);
				next();
			});
		});

		// CMC specific endpoint
		const readAndParseNetworkPropertiesFile = () => {
			const readFile = util.promisify(fs.readFile);
			return readFile(services.config.apiNode.networkPropertyFilePath, 'utf8')
				.then(fileData => ini.parse(fileData));
		};

		server.get('/network/currency/circulating', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(propertiesObject => {
				const currencyId = propertiesObject.chain.currencyMosaicId.replace(/'/g, '').replace('0x', '');
				const mosaicId = routeUtils.parseArgument({ mosaicId: currencyId }, 'mosaicId', 'uint64hex');
				return db.mosaicsByIds([mosaicId]).then(response => {
					const s = response[0].mosaic.supply.toString();
					const supply = `${s.substring(0, s.length - 6)}.${s.substring(s.length - 6, s.length)}`;
					res.setHeader('content-type', 'text/plain');
					res.send(supply);
					next();
				});
			}));
	}
};
