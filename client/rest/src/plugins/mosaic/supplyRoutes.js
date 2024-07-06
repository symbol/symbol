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
import { longToUint64 } from '../../db/dbUtils.js';
import { sendPlainText } from '../../routes/simpleSend.js';
import AccountType from '../AccountType.js';
import ini from 'ini';
import { utils } from 'symbol-sdk';

const fileLoader = new catapult.utils.CachedFileLoader();

export default {
	register: (server, db, services) => {
		const convertToFractionalWholeUnits = (value, divisibility) => (Number(value) / (10 ** divisibility)).toFixed(divisibility);

		const propertyValueToMosaicId = value => BigInt(value.replace(/'/g, ''));

		const readAndParseNetworkPropertiesFile = () => fileLoader.readOnce(
			services.config.apiNode.networkPropertyFilePath,
			contents => ini.parse(contents)
		);

		const getMosaicProperties = async currencyMosaicId => {
			const mosaics = await db.mosaicsByIds([currencyMosaicId]);
			return {
				totalSupply: mosaics[0].mosaic.supply.toNumber(),
				divisibility: mosaics[0].mosaic.divisibility
			};
		};

		const getUncirculatingAccountIds = propertiesObject => {
			const publicKeys = [propertiesObject.network.nemesisSignerPublicKey].concat(services.config.uncirculatingAccountPublicKeys);
			return publicKeys.map(publicKey => ({ [AccountType.publicKey]: utils.hexToUint8(publicKey) }));
		};

		const lookupMosaicAmount = (mosaics, currencyMosaicId) => {
			const matchingMosaic = mosaics.find(mosaic => {
				const mosaicId = longToUint64(mosaic.id); // convert Long to bigint
				return currencyMosaicId === mosaicId;
			});

			return undefined === matchingMosaic ? 0 : matchingMosaic.amount.toNumber();
		};

		server.get('/network/currency/supply/circulating', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(async propertiesObject => {
				const currencyMosaicId = propertyValueToMosaicId(propertiesObject.chain.currencyMosaicId);
				const currencyMosaicProperties = await getMosaicProperties(currencyMosaicId);

				const accounts = await db.catapultDb.accountsByIds(getUncirculatingAccountIds(propertiesObject));
				const burnedSupply = accounts.reduce(
					(sum, account) => sum + lookupMosaicAmount(account.account.mosaics, currencyMosaicId),
					0
				);

				sendPlainText(res, next)(convertToFractionalWholeUnits(
					currencyMosaicProperties.totalSupply - burnedSupply,
					currencyMosaicProperties.divisibility
				));
			}));

		server.get('/network/currency/supply/total', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(async propertiesObject => {
				const currencyMosaicId = propertyValueToMosaicId(propertiesObject.chain.currencyMosaicId);
				const currencyMosaicProperties = await getMosaicProperties(currencyMosaicId);
				sendPlainText(res, next)(convertToFractionalWholeUnits(
					currencyMosaicProperties.totalSupply,
					currencyMosaicProperties.divisibility
				));
			}));

		server.get('/network/currency/supply/max', (req, res, next) => readAndParseNetworkPropertiesFile()
			.then(async propertiesObject => {
				const currencyMosaicId = propertyValueToMosaicId(propertiesObject.chain.currencyMosaicId);
				const currencyMosaicProperties = await getMosaicProperties(currencyMosaicId);

				const maxSupply = parseInt(propertiesObject.chain.maxMosaicAtomicUnits.replace(/'/g, ''), 10);
				sendPlainText(res, next)(convertToFractionalWholeUnits(maxSupply, currencyMosaicProperties.divisibility));
			}));
	}
};
