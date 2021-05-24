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

const cmcUtils = require('./cmcUtils');
const uncirculatedAddresses = require('./unCirculatedAccounts');
const routeUtils = require('../../routes/routeUtils');
const errors = require('../../server/errors');
const AccountType = require('../AccountType');
const ini = require('ini');
const fs = require('fs');
const util = require('util');

module.exports = {
	register: (server, db, services) => {
		const sender = routeUtils.createSender('cmc');

		const readAndParseNetworkPropertiesFile = () => {
			const readFile = util.promisify(fs.readFile);
			return readFile(services.config.apiNode.networkPropertyFilePath, 'utf8')
				.then(fileData => ini.parse(fileData));
		};

		server.get('/network/currency/supply/circulating', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(async propertiesObject => {
				/* eslint-disable global-require */
				const accountIds = routeUtils.parseArgumentAsArray({ addresses: uncirculatedAddresses }, 'addresses', 'address');
				const currencyId = propertiesObject.chain.currencyMosaicId.replace(/'/g, '').replace('0x', '');
				const mosaicId = routeUtils.parseArgument({ mosaicId: currencyId }, 'mosaicId', 'uint64hex');

				const mosaics = await db.mosaicsByIds([mosaicId]);
				const accounts = await db.catapultDb.accountsByIds(accountIds.map(accountId => ({ [AccountType.address]: accountId })));

				const totalSupply = parseInt(mosaics[0].mosaic.supply.toString(), 10);
				const totalUncirculated = accounts.reduce((a, b) => a + parseInt(b.account.mosaics[0].amount.toString(), 10), 0);

				const circulatingSupply = (totalSupply - totalUncirculated).toString();

				sender.sendPlainText(res, next)(cmcUtils.convertToRelative(circulatingSupply));
			}).catch(() => {
				res.send(errors.createInvalidArgumentError('there was an error reading the network properties file'));
				next();
			}));

		server.get('/network/currency/supply/total', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(propertiesObject => {
				const currencyId = propertiesObject.chain.currencyMosaicId.replace(/'/g, '').replace('0x', '');
				const mosaicId = routeUtils.parseArgument({ mosaicId: currencyId }, 'mosaicId', 'uint64hex');
				return db.mosaicsByIds([mosaicId]).then(response => {
					const supply = response[0].mosaic.supply.toString();

					sender.sendPlainText(res, next)(cmcUtils.convertToRelative(supply));
				}).catch(() => {
					res.send(errors.createInvalidArgumentError('there was an error reading the network properties file'));
					next();
				});
			}));

		server.get('/network/currency/supply/max', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(propertiesObject => {
				const supply = propertiesObject.chain.maxMosaicAtomicUnits.replace(/'/g, '').replace('0x', '');
				sender.sendPlainText(res, next)(cmcUtils.convertToRelative(supply));
			}).catch(() => {
				res.send(errors.createInvalidArgumentError('there was an error reading the network properties file'));
				next();
			}));
	}
};
