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

import { mosaicIdToString } from './rosettaUtils.js';
import { RosettaErrorFactory } from '../rosettaUtils.js';
import { models } from 'symbol-sdk/nem';

/**
 * Proxy to a NEM node that performs caching for performance optimization, as appropriate.
 */
export default class NemProxy {
	/**
	 * Creates a proxy around an endpoint.
	 * @param {string} endpoint NEM endpoint.
	 */
	constructor(endpoint) {
		this.endpoint = endpoint;

		this.mosaicPropertiesMap = new Map();
	}

	/**
	 * Performs an (uncached) fetch request on the endpoint.
	 * @param {string} urlPath Request path.
	 * @param {Function} jsonProjection Processes result.
	 * @param {object} requestOptions Additional fetch request options.
	 * @returns {object} Result of fetch and projection.
	 */
	async fetch(urlPath, jsonProjection = undefined, requestOptions = {}) {
		try {
			const response = await global.fetch(`${this.endpoint}/${urlPath}`, requestOptions);
			if (!response.ok)
				throw RosettaErrorFactory.CONNECTION_ERROR;

			const jsonObject = await response.json();
			return jsonProjection ? jsonProjection(jsonObject) : jsonObject;
		} catch (err) {
			throw RosettaErrorFactory.CONNECTION_ERROR;
		}
	}

	/**
	 * Performs an (uncached) chain of fetch requests on the endpoint.
	 * @param {string} urlPath Request path.
	 * @param {number} pageSize Page size.
	 * @param {Function} jsonProjection Processes result.
	 * @param {object} requestOptions Additional fetch request options.
	 * @returns {object} Result of fetch and projection.
	 */
	async fetchAll(urlPath, pageSize, jsonProjection = undefined, requestOptions = {}) {
		const jsonObjects = [];
		let response;
		do {
			const delimiter = -1 !== urlPath.indexOf('?') ? '&' : '?';
			const idFilter = 0 === jsonObjects.length ? '' : `id=${jsonObjects[jsonObjects.length - 1].meta.id}&`;
			const nextUrlPath = `${urlPath}${delimiter}${idFilter}pageSize=${pageSize}`;

			// eslint-disable-next-line no-await-in-loop
			response = await this.fetch(nextUrlPath, jsonObject => jsonObject.data, requestOptions);
			jsonObjects.push(...response);
		} while (response.length === pageSize);

		return jsonProjection ? jsonObjects.map(jsonProjection) : jsonObjects;
	}

	/**
	 * Retrieves (uncached) block at height.
	 * @param {number} height Block height.
	 * @returns {Promise<object>} Block data.
	 */
	async localBlockAtHeight(height) {
		const block = await this.fetch('local/block/at', undefined, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify({ height })
		});
		block.hash = block.hash.toUpperCase();
		return block;
	}

	/**
	 * Retrieves (potentially cached) mosaic properties.
	 * @param {object} mosaicId NEM mosaic id object.
	 * @returns {object} Properties about the mosaic.
	 */
	async mosaicProperties(mosaicId) {
		const fullyQualifiedName = mosaicIdToString(mosaicId);
		if (this.mosaicPropertiesMap.has(fullyQualifiedName))
			return this.mosaicPropertiesMap.get(fullyQualifiedName);

		const mosaicDefinition = await this.fetch(`mosaic/definition/last?mosaicId=${mosaicId.namespaceId}:${mosaicId.name}`);

		const findProperty = (properties, name) => properties.find(property => name === property.name);
		const divisibilityProperty = findProperty(mosaicDefinition.properties, 'divisibility');

		const { levy } = mosaicDefinition;
		const properties = {
			divisibility: undefined === divisibilityProperty ? 0 : parseInt(divisibilityProperty.value, 10),
			levy: undefined === mosaicDefinition.levy.type ? undefined : {
				mosaicId: levy.mosaicId,
				recipientAddress: levy.recipient,
				isAbsolute: models.MosaicTransferFeeType.ABSOLUTE.value === levy.type,
				fee: levy.fee
			}
		};
		this.mosaicPropertiesMap.set(fullyQualifiedName, properties);
		return properties;
	}
}
