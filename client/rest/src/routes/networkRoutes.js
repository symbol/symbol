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

import catapult from '../catapult-sdk/index.js';
import errors from '../server/errors.js';
import ini from 'ini';

const fileLoader = new catapult.utils.CachedFileLoader();

export default {
	register: (server, db, services) => {
		const average = array => array.reduce((p, c) => p + c, 0) / array.length;
		const median = array => {
			array.sort((a, b) => a - b);
			const mid = array.length / 2;
			return mid % 1 ? array[mid - 0.5] : (array[mid - 1] + array[mid]) / 2;
		};

		const readAndParseNetworkPropertiesFile = () => fileLoader.readOnce(
			services.config.apiNode.networkPropertyFilePath,
			contents => {
				const propertiesObject = ini.parse(contents);
				propertiesObject.parsed = {};

				if (propertiesObject.treasury_reissuance_transaction_signatures) {
					const signaturesMap = propertiesObject.treasury_reissuance_transaction_signatures;
					propertiesObject.parsed.treasuryReissuanceTransactionSignatures = Object.keys(signaturesMap)
						.filter(key => signaturesMap[key])
						.sort();
				}

				if (propertiesObject.corrupt_aggregate_transaction_hashes) {
					const hashesMap = propertiesObject.corrupt_aggregate_transaction_hashes;
					propertiesObject.parsed.corruptAggregateTransactionHashes = Object.keys(hashesMap)
						.map(key => `${key} = ${hashesMap[key]}`)
						.sort();
				}

				return propertiesObject;
			}
		);

		const readAndParseNodePropertiesFile = () => fileLoader.readNewer(
			services.config.apiNode.nodePropertyFilePath,
			contents => ini.parse(contents)
		);

		const readAndParseInflationPropertiesFile = () => fileLoader.readOnce(
			services.config.apiNode.inflationPropertyFilePath,
			contents => {
				const inflationObject = ini.parse(contents).inflation;
				const inflationInflectionPoints = Object.getOwnPropertyNames(inflationObject).map(key => ({
					startHeight: BigInt(key.substring(key.lastIndexOf('-') + 1)),
					rewardAmount: BigInt(inflationObject[key])
				}));

				// sort by start height
				inflationInflectionPoints.sort((lhs, rhs) => {
					if (lhs.startHeight === rhs.startHeight)
						return 0;

					return lhs.startHeight > rhs.startHeight ? 1 : -1;
				});
				return inflationInflectionPoints;
			}
		);

		const sanitizeInput = value => value.replace(/[^0-9]/g, '');

		server.get('/network', async (request, reply) => {
			const { name, description } = services.config.network;
			return reply.send({ name, description });
		});

		server.get('/network/properties', async (request, reply) => {
			try {
				const propertiesObject = await readAndParseNetworkPropertiesFile();
				const networkProperties = {
					network: propertiesObject.network,
					chain: propertiesObject.chain,
					plugins: propertiesObject['plugin:catapult'].plugins,
					forkHeights: propertiesObject.fork_heights
				};

				if (propertiesObject.treasury_reissuance_transaction_signatures) {
					const key = 'treasuryReissuanceTransactionSignatures';
					networkProperties[key] = propertiesObject.parsed[key];
				}

				if (propertiesObject.corrupt_aggregate_transaction_hashes) {
					const key = 'corruptAggregateTransactionHashes';
					networkProperties[key] = propertiesObject.parsed[key];
				}

				return reply.send(networkProperties);
			} catch {
				throw errors.createInvalidArgumentError('there was an error reading the network properties file');
			}
		});

		server.get('/network/inflation', async (request, reply) => {
			try {
				const inflationInflectionPoints = await readAndParseInflationPropertiesFile();
				return reply.send(inflationInflectionPoints.map(point => ({
					// send BigInts over network as strings
					startHeight: point.startHeight.toString(),
					rewardAmount: point.rewardAmount.toString()
				})));
			} catch {
				throw errors.createInvalidArgumentError('there was an error reading the inflation properties file');
			}
		});

		server.get('/network/inflation/at/:height', async (request, reply) => {
			try {
				const inflationInflectionPoints = await readAndParseInflationPropertiesFile();
				const height = BigInt(request.params.height);

				const findMatchingPoint = () => {
					const firstPoint = inflationInflectionPoints[0];
					if (height < firstPoint.startHeight)
						return { startHeight: 'N/A', rewardAmount: '0' };

					for (let i = 1; i < inflationInflectionPoints.length; ++i) {
						if (height < inflationInflectionPoints[i].startHeight)
							return inflationInflectionPoints[i - 1];
					}

					return inflationInflectionPoints[inflationInflectionPoints.length - 1];
				};

				const point = findMatchingPoint();
				return reply.send({
					startHeight: point.startHeight.toString(),
					rewardAmount: point.rewardAmount.toString()
				});
			} catch {
				throw errors.createInvalidArgumentError('there was an error reading the inflation properties file');
			}
		});

		server.get('/network/fees/transaction', async (request, reply) => {
			const numBlocksTransactionFeeStats = services.config.numBlocksTransactionFeeStats || 1;
			const latestBlocksFeeMultiplier = db.latestBlocksFeeMultiplier(numBlocksTransactionFeeStats);
			const feeMultipliers = await Promise.all([
				readAndParseNodePropertiesFile(), latestBlocksFeeMultiplier, readAndParseNetworkPropertiesFile()
			]);
			// defaultDynamicFeeMultiplier -> uint32
			const defaultDynamicFeeMultiplier = parseInt(sanitizeInput(feeMultipliers[2].chain.defaultDynamicFeeMultiplier), 10);
			const defaultedFeeMultipliers = feeMultipliers[1].map(f => (0 === f ? defaultDynamicFeeMultiplier : f));

			return reply.send({
				averageFeeMultiplier: Math.floor(average(defaultedFeeMultipliers)),
				medianFeeMultiplier: Math.floor(median(defaultedFeeMultipliers)),
				highestFeeMultiplier: Math.max(...feeMultipliers[1]),
				lowestFeeMultiplier: Math.min(...feeMultipliers[1]),
				minFeeMultiplier: Number(feeMultipliers[0].node.minFeeMultiplier.replace(/'/g, ''))
			});
		});

		server.get('/network/fees/rental', async (request, reply) => {
			try {
				const propertiesObject = await readAndParseNetworkPropertiesFile();
				const maxDifficultyBlocks = parseInt(sanitizeInput(propertiesObject.chain.maxDifficultyBlocks), 10);

				// defaultDynamicFeeMultiplier -> uint32
				const defaultDynamicFeeMultiplier = parseInt(sanitizeInput(propertiesObject.chain.defaultDynamicFeeMultiplier), 10);

				// rootNamespaceRentalFeePerBlock -> bigint
				const lookupPluginPropertyUint64 = (pluginName, propertyName) => {
					const rawPropertyValue = propertiesObject['plugin:catapult'].plugins[pluginName][propertyName];
					return BigInt(sanitizeInput(rawPropertyValue));
				};
				const rootNamespaceRentalFeePerBlock = lookupPluginPropertyUint64('namespace', 'rootNamespaceRentalFeePerBlock');

				// childNamespaceRentalFee -> bigint
				const childNamespaceRentalFee = lookupPluginPropertyUint64('namespace', 'childNamespaceRentalFee');

				// mosaicRentalFee -> bigint
				const mosaicRentalFee = lookupPluginPropertyUint64('mosaic', 'mosaicRentalFee');

				const feeMultipliers = await db.latestBlocksFeeMultiplier(maxDifficultyBlocks || 1);
				const defaultedFeeMultipliers = feeMultipliers.map(f => (0 === f ? defaultDynamicFeeMultiplier : f));
				const medianNetworkMultiplier = Math.floor(median(defaultedFeeMultipliers));
				const uint64MedianNetworkMultiplier = BigInt(medianNetworkMultiplier);

				return reply.send({
					effectiveRootNamespaceRentalFeePerBlock: (rootNamespaceRentalFeePerBlock * uint64MedianNetworkMultiplier).toString(),
					effectiveChildNamespaceRentalFee: (childNamespaceRentalFee * uint64MedianNetworkMultiplier).toString(),
					effectiveMosaicRentalFee: (mosaicRentalFee * uint64MedianNetworkMultiplier).toString()
				});
			} catch {
				throw errors.createInvalidArgumentError('there was an error reading the network properties file');
			}
		});
	}
};
