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

const errors = require('../server/errors');
const ini = require('ini');
const fs = require('fs');
const util = require('util');

module.exports = {
	register: (server, db, services) => {
		server.get('/network', (req, res, next) => {
			res.send({ name: services.config.network.name, description: services.config.network.description });
			next();
		});

		server.get('/network/properties', (req, res, next) => {
			const readFile = util.promisify(fs.readFile);
			return readFile(services.config.network.propertiesFilePath, 'utf8')
				.then(fileData => ini.parse(fileData))
				.then(propertiesObject => {
					res.send({
						network: propertiesObject.network,
						chain: propertiesObject.chain,
						plugins: propertiesObject['plugin:catapult'].plugins
					});
					next();
				})
				.catch(() => {
					res.send(errors.createInvalidArgumentError('there was an error reading the network properties file'));
					next();
				});
		});

		server.get('/network/fees/transaction', (req, res, next) => {
			const average = array => array.reduce((p, c) => p + c, 0) / array.length;
			const median = array => {
				array.sort((a, b) => a - b);
				const mid = array.length / 2;
				return mid % 1 ? array[mid - 0.5] : (array[mid - 1] + array[mid]) / 2;
			};
			const round = num => Number(num.toFixed(2));

			const numBlocksTransactionFeeStats = services.config.numBlocksTransactionFeeStats || 1;
			return db.latestBlocksFeeMultiplier(numBlocksTransactionFeeStats).then(feeMultipliers => {
				res.send({
					averageFeeMultiplier: round(average(feeMultipliers)),
					medianFeeMultiplier: round(median(feeMultipliers)),
					highestFeeMultiplier: Math.max(...feeMultipliers),
					lowestFeeMultiplier: Math.min(...feeMultipliers)
				});
				next();
			});
		});
	}
};
