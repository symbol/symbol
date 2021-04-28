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

const merkleUtils = require('./merkleUtils');
const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');
const AccountType = require('../plugins/AccountType');
const MosaicDb = require('../plugins/mosaic/MosaicDb');
const errors = require('../server/errors');
const catapult = require('catapult-sdk');
const ini = require('ini');
const fs = require('fs');
const util = require('util');

const { PacketType } = catapult.packet;

module.exports = {
	register: (server, db, services) => {
		const sender = routeUtils.createSender(routeResultTypes.account);

		server.get('/accounts', (req, res, next) => {
			const address = req.params.address ? routeUtils.parseArgument(req.params, 'address', 'address') : undefined;
			const mosaicId = req.params.mosaicId ? routeUtils.parseArgument(req.params, 'mosaicId', 'uint64hex') : undefined;

			const offsetParsers = {
				id: 'objectId',
				balance: 'uint64'
			};
			const options = routeUtils.parsePaginationArguments(req.params, services.config.pageSize, offsetParsers);

			if ('balance' === options.sortField && !mosaicId)
				throw errors.createInvalidArgumentError('mosaicId must be provided when sorting by balance');

			return db.accounts(address, mosaicId, options)
				.then(result => sender.sendPage(res, next)(result));
		});

		server.get('/accounts/:accountId', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			return db.accountsByIds([{ [type]: accountId }])
				.then(sender.sendOne(req.params.accountId, res, next));
		});

		server.post('/accounts', (req, res, next) => {
			if (req.params.publicKeys && req.params.addresses)
				throw errors.createInvalidArgumentError('publicKeys and addresses cannot both be provided');

			const idOptions = Array.isArray(req.params.publicKeys)
				? { keyName: 'publicKeys', parserName: 'publicKey', type: AccountType.publicKey }
				: { keyName: 'addresses', parserName: 'address', type: AccountType.address };

			const accountIds = routeUtils.parseArgumentAsArray(req.params, idOptions.keyName, idOptions.parserName);

			return db.accountsByIds(accountIds.map(accountId => ({ [idOptions.type]: accountId })))
				.then(sender.sendArray(idOptions.keyName, res, next));
		});

		// this endpoint is here because it is expected to support requests by block other than <current block>
		server.get('/accounts/:accountId/merkle', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');
			const encodedAddress = 'publicKey' === type ? catapult.model.address.publicKeyToAddress(accountId, db.networkId) : accountId;
			const state = PacketType.accountStatePath;
			return merkleUtils.requestTree(services, state,
				encodedAddress).then(response => {
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

		// CMC specific endpoint
		server.get('/network/currency/supply/circulating', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(async propertiesObject => {
				const mosaicDB = new MosaicDb(db);
				/* eslint-disable global-require */
				const uncirculatedAddresses = require('../constants/unCirculatedAccounts');
				const accountIds = routeUtils.parseArgumentAsArray({ addresses: uncirculatedAddresses }, 'addresses', 'address');
				const currencyId = propertiesObject.chain.currencyMosaicId.replace(/'/g, '').replace('0x', '');
				const mosaicId = routeUtils.parseArgument({ mosaicId: currencyId }, 'mosaicId', 'uint64hex');

				const mosaics = await mosaicDB.mosaicsByIds([mosaicId]);
				const accounts = await db.accountsByIds(accountIds.map(accountId => ({ [AccountType.address]: accountId })));

				const totalSupply = parseInt(mosaics[0].mosaic.supply.toString(), 10);
				const totalUncirculated = accounts.reduce((a, b) => a + parseInt(b.account.mosaics[0].amount.toString(), 10), 0);

				const s = (totalSupply - totalUncirculated).toString();
				const result = `${s.substring(0, s.length - 6)}.${s.substring(s.length - 6, s.length)}`;
				res.setHeader('content-type', 'text/plain');
				res.send(result);
				next();
			}));
	}
};
