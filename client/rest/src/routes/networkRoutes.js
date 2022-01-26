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

const catapult = require('../catapult-sdk/index');
const errors = require('../server/errors');
const ini = require('ini');
const fs = require('fs');
const util = require('util');

const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db, services) => {
		const average = array => array.reduce((p, c) => p + c, 0) / array.length;
		const median = array => {
			array.sort((a, b) => a - b);
			const mid = array.length / 2;
			return mid % 1 ? array[mid - 0.5] : (array[mid - 1] + array[mid]) / 2;
		};

		const readAndParseNetworkPropertiesFile = () => {
			const readFile = util.promisify(fs.readFile);
			return readFile(services.config.apiNode.networkPropertyFilePath, 'utf8')
				.then(fileData => ini.parse(fileData));
		};

		const readAndParseNodePropertiesFile = () => {
			const readFile = util.promisify(fs.readFile);
			return readFile(services.config.apiNode.nodePropertyFilePath, 'utf8')
				.then(fileData => ini.parse(fileData));
		};

		const sanitizeInput = value => value.replace(/[^0-9]/g, '');

		server.get('/network', (req, res, next) => {
			res.send({ name: services.config.network.name, description: services.config.network.description });
			next();
		});

		server.get('/network/properties', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(propertiesObject => {
				res.send({
					network: propertiesObject.network,
					chain: propertiesObject.chain,
					plugins: propertiesObject['plugin:catapult'].plugins
				});
				next();
			}).catch(() => {
				res.send(errors.createInvalidArgumentError('there was an error reading the network properties file'));
				next();
			}));

		server.get('/network/fees/transaction', (req, res, next) => {
			const numBlocksTransactionFeeStats = services.config.numBlocksTransactionFeeStats || 1;
			const latestBlocksFeeMultiplier = db.latestBlocksFeeMultiplier(numBlocksTransactionFeeStats);
			return Promise.all([readAndParseNodePropertiesFile(), latestBlocksFeeMultiplier,
				readAndParseNetworkPropertiesFile()]).then(feeMultipliers => {
				// defaultDynamicFeeMultiplier -> uint32
				const defaultDynamicFeeMultiplier = parseInt(sanitizeInput(feeMultipliers[2].chain.defaultDynamicFeeMultiplier), 10);
				const defaultedFeeMultipliers = feeMultipliers[1].map(f => (0 === f ? defaultDynamicFeeMultiplier : f));

				res.send({
					averageFeeMultiplier: Math.floor(average(defaultedFeeMultipliers)),
					medianFeeMultiplier: Math.floor(median(defaultedFeeMultipliers)),
					highestFeeMultiplier: Math.max(...feeMultipliers[1]),
					lowestFeeMultiplier: Math.min(...feeMultipliers[1]),
					minFeeMultiplier: Number(feeMultipliers[0].node.minFeeMultiplier.replace(/'/g, ''))
				});
				next();
			});
		});

		server.get('/network/fees/rental', (req, res, next) => readAndParseNetworkPropertiesFile().then(propertiesObject => {
			const maxDifficultyBlocks = parseInt(sanitizeInput(propertiesObject.chain.maxDifficultyBlocks), 10);

			// defaultDynamicFeeMultiplier -> uint32
			const defaultDynamicFeeMultiplier = parseInt(sanitizeInput(propertiesObject.chain.defaultDynamicFeeMultiplier), 10);

			// rootNamespaceRentalFeePerBlock -> uint64
			const lookupPluginPropertyUint64 = (pluginName, propertyName) => {
				const rawPropertyValue = propertiesObject['plugin:catapult'].plugins[pluginName][propertyName];
				return uint64.fromString(sanitizeInput(rawPropertyValue));
			};
			const rootNamespaceRentalFeePerBlock = lookupPluginPropertyUint64('namespace', 'rootNamespaceRentalFeePerBlock');

			// childNamespaceRentalFee -> uint64
			const childNamespaceRentalFee = lookupPluginPropertyUint64('namespace', 'childNamespaceRentalFee');

			// mosaicRentalFee -> uint64
			const mosaicRentalFee = lookupPluginPropertyUint64('mosaic', 'mosaicRentalFee');

			return db.latestBlocksFeeMultiplier(maxDifficultyBlocks || 1).then(feeMultipliers => {
				const defaultedFeeMultipliers = feeMultipliers.map(f => (0 === f ? defaultDynamicFeeMultiplier : f));
				const medianNetworkMultiplier = Math.floor(median(defaultedFeeMultipliers));
				const uint64MedianNetworkMultiplier = uint64.fromUint(medianNetworkMultiplier);

				res.send({
					effectiveRootNamespaceRentalFeePerBlock:
						uint64.toString(uint64.multiply(rootNamespaceRentalFeePerBlock, uint64MedianNetworkMultiplier)),
					effectiveChildNamespaceRentalFee:
						uint64.toString(uint64.multiply(childNamespaceRentalFee, uint64MedianNetworkMultiplier)),
					effectiveMosaicRentalFee:
						uint64.toString(uint64.multiply(mosaicRentalFee, uint64MedianNetworkMultiplier))
				});
				next();
			});
		}).catch(() => {
			res.send(errors.createInvalidArgumentError('there was an error reading the network properties file'));
			next();
		}));
	}
};
