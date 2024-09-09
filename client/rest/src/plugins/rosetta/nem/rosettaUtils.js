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
import { Network } from 'symbol-sdk/nem';

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
export const mosaicIdToString = mosaicId => `${mosaicId.namespaceId}:${mosaicId.name}`;

/**
 * Calculates NEM XEM transfer fee.
 * @param {bigint} amount XEM transfer amount.
 * @returns {bigint} XEM transfer fee.
 */
export const calculateXemTransferFee = amount => {
	// Math.min and Math.max don't work with bigint
	const min = (lhs, rhs) => (lhs < rhs ? lhs : rhs);
	const max = (lhs, rhs) => (lhs > rhs ? lhs : rhs);
	return 50000n * min(25n, max(1n, amount / 10000n));
};

const NEM_XEM_DIVISIBILITY = 6;

const isNemXemMosaicId = mosaicId => 'nem' === mosaicId.namespaceId && 'xem' === mosaicId.name;

const lookupDivisibility = async (mosaicId, transactionLocation, proxy) => {
	if (isNemXemMosaicId(mosaicId))
		return NEM_XEM_DIVISIBILITY;

	const mosaicProperties = await proxy.mosaicProperties(mosaicId, transactionLocation);
	return mosaicProperties.divisibility;
};

/**
 * Creates the lookup currency function used by the operation parser.
 * @param {object} proxy NEM proxy.
 * @returns {Function} Currency lookup function.
 */
export const createLookupCurrencyFunction = proxy => async (mosaicId, transactionLocation) => {
	if ('currencyMosaicId' === mosaicId || isNemXemMosaicId(mosaicId))
		return { currency: new Currency('nem:xem', NEM_XEM_DIVISIBILITY) };

	const mosaicProperties = await proxy.mosaicProperties(mosaicId, transactionLocation);
	const currency = new Currency(mosaicIdToString(mosaicId), mosaicProperties.divisibility);

	if (undefined === mosaicProperties.levy)
		return { currency };

	const levyDivisibility = await lookupDivisibility(mosaicProperties.levy.mosaicId, transactionLocation, proxy);
	const result = {
		currency,
		levy: {
			...mosaicProperties.levy,
			currency: new Currency(mosaicIdToString(mosaicProperties.levy.mosaicId), levyDivisibility)
		}
	};
	delete result.levy.mosaicId;
	return result;
};

/**
 * Checks if two mosaic ids (REST JSON model) are equal.
 * @param {object} lhs First mosaic id.
 * @param {object} rhs Second mosaic id.
 * @returns {boolean} \c true if mosaic ids are equal.
 */
export const areMosaicIdsEqual = (lhs, rhs) => lhs.namespaceId === rhs.namespaceId && lhs.name === rhs.name;

/**
 * Checks if two mosaic definitions (REST JSON model) are equal at specified height.
 * @param {object} lhs First mosaic definition.
 * @param {object} rhs Second mosaic definition.
 * @param {boolean} isDescriptionSignificant \c true if description differences are significant.
 * @returns {boolean} \c true if mosaic definitions are equal.
 */
export const areMosaicDefinitionsEqual = (lhs, rhs, isDescriptionSignificant) => {
	if (isDescriptionSignificant) {
		if (lhs.description !== rhs.description)
			return false;
	}

	const isPropertyEqual = name => {
		const lhsProperty = lhs.properties.find(property => name === property.name);
		const rhsProperty = rhs.properties.find(property => name === property.name);
		return lhsProperty.value === rhsProperty.value;
	};

	const normalizeObject = object => (undefined === object ? {} : object);

	const lhsLevy = normalizeObject(lhs.levy);
	const rhsLevy = normalizeObject(rhs.levy);

	const lhsLevyMosaicId = normalizeObject(lhsLevy.mosaicId);
	const rhsLevyMosaicId = normalizeObject(rhsLevy.mosaicId);

	return lhs.creator === rhs.creator
		&& lhs.properties.every(property => isPropertyEqual(property.name))
		&& lhsLevy.fee === rhsLevy.fee
		&& lhsLevy.recipient === rhsLevy.recipient
		&& lhsLevy.type === rhsLevy.type
		&& areMosaicIdsEqual(lhsLevyMosaicId, rhsLevyMosaicId);
};

/**
 * Determines if mosaic definition description differences are significant given a network and transaction location.
 * @param {Network} network Blockchain network.
 * @param {object} transactionLocation Location of transaction.
 * @returns {boolean} true if mosaic definition description differences are significant.
 */
export const isMosaicDefinitionDescriptionSignificant = (network, transactionLocation) => {
	const forkHeight = (() => {
		if ('mainnet' === network.name)
			return 1_110_000;

		return 'testnet' === network.name ? 871_500 : 1;
	})();

	return undefined !== transactionLocation && undefined !== transactionLocation.height && transactionLocation.height < forkHeight;
};
