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

import Currency from '../openApi/model/Currency.js';

/**
 * Extracts blockchain descriptor from services configuration.
 * @param {object} config Services configuration.
 * @returns {object} Blockchain descriptor.
 */
export const getBlockchainDescriptor = config => ({ // eslint-disable-line import/prefer-default-export
	blockchain: 'NEM',
	network: config.network.name
});

/**
 * Converts a NEM mosaic id object into a string.
 * @param {object} mosaicId NEM mosaic id object.
 * @returns {string} Fully qualified mosaic name.
 */
export const mosaicIdToString = mosaicId => `${mosaicId.namespaceId}.${mosaicId.name}`;

/**
 * Creates the lookup currency function used by the operation parser.
 * @param {object} proxy NEM proxy.
 * @returns {Function} Currency lookup function.
 */
export const createLookupCurrencyFunction = proxy => async mosaicId => {
	if ('currencyMosaicId' === mosaicId)
		return { currency: new Currency('nem.xem', 6) };

	const mosaicProperties = await proxy.mosaicProperties(mosaicId);
	const currency = new Currency(mosaicIdToString(mosaicId), mosaicProperties.divisibility);

	if (undefined === mosaicProperties.levy)
		return { currency };

	const levyMosaicProperties = await proxy.mosaicProperties(mosaicProperties.levy.mosaicId);
	const result = {
		currency,
		levy: {
			...mosaicProperties.levy,
			currency: new Currency(mosaicIdToString(mosaicProperties.levy.mosaicId), levyMosaicProperties.divisibility)
		}
	};
	delete result.levy.mosaicId;
	return result;
};
