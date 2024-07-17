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

import { RosettaErrorFactory } from './rosettaUtils.js';

const bigIntToHexString = value => value.toString(16).toUpperCase();

/**
 * Proxy to a catapult node that performs caching for performance optimization, as appropriate.
 */
export default class CatapultProxy {
	/**
	 * Creates a proxy around an endpoint.
	 * @param {string} endpoint Catapult endpoint.
	 */
	constructor(endpoint) {
		this.endpoint = endpoint;

		this.cache = undefined;
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
		let pageNumber = 1;
		let response;
		do {
			const delimiter = -1 !== urlPath.indexOf('?') ? '&' : '?';
			const nextUrlPath = `${urlPath}${delimiter}pageNumber=${pageNumber}&pageSize=${pageSize}`;
			pageNumber++;

			// eslint-disable-next-line no-await-in-loop
			response = await this.fetch(nextUrlPath, undefined, requestOptions);
			jsonObjects.push(...response);
		} while (response.length === pageSize);

		return jsonProjection ? jsonObjects.map(jsonProjection) : jsonObjects;
	}

	/**
	 * Reads property from cache.
	 * @param {string} propertyName Property name.
	 * @returns {object} Property value.
	 * @private
	 */
	async readCacheProperty(propertyName) {
		if (!this.cache) {
			const results = await Promise.all([
				this.fetch('node/info'),
				this.fetch('network/properties'),
				this.fetch('blocks/1')
			]);

			this.cache = {
				nodeInfo: results[0],
				networkProperties: results[1],
				nemesisBlock: results[2]
			};
		}

		return this.cache[propertyName];
	}

	/**
	 * Gets (potentially cached) catapult node information.
	 * @returns {object} Catapult node information.
	 */
	nodeInfo() {
		return this.readCacheProperty('nodeInfo');
	}

	/**
	 * Gets (potentially cached) catapult network information.
	 * @returns {object} Catapult network information.
	 */
	networkProperties() {
		return this.readCacheProperty('networkProperties');
	}

	/**
	 * Gets (potentially cached) catapult nemesis block.
	 * @returns {object} Catapult nemesis block.
	 */
	nemesisBlock() {
		return this.readCacheProperty('nemesisBlock');
	}

	/**
	 * Resolves a mosaic id.
	 * @param {bigint} unresolvedMosaicId Unresolved mosaic id.
	 * @param {object} transactionLocation Location of transaction for which to perform the resolution.
	 * @returns {bigint} Resolved mosaic id.
	 */
	async resolveMosaicId(unresolvedMosaicId, transactionLocation = undefined) {
		if (0n === (unresolvedMosaicId & (1n << 63n)))
			return unresolvedMosaicId;

		if (!transactionLocation) {
			const mosaicIdHexString = await this.fetch(
				`namespaces/${bigIntToHexString(unresolvedMosaicId)}`,
				jsonObject => jsonObject.namespace.alias.mosaicId
			);
			return BigInt(`0x${mosaicIdHexString}`);
		}

		const statements = await this.fetch(
			`statements/resolutions/mosaic?height=${transactionLocation.height}`,
			jsonObject => jsonObject.data
		);
		const resolutionStatement = statements.find(statement => unresolvedMosaicId === BigInt(`0x${statement.statement.unresolved}`));
		if (!resolutionStatement)
			throw RosettaErrorFactory.INTERNAL_SERVER_ERROR;

		for (let i = resolutionStatement.statement.resolutionEntries.length - 1; 0 <= i; --i) {
			const resolutionEntry = resolutionStatement.statement.resolutionEntries[i];
			const { source } = resolutionEntry;
			if (source.primaryId <= transactionLocation.primaryId && source.secondaryId <= transactionLocation.secondaryId)
				return BigInt(`0x${resolutionEntry.resolved}`);
		}

		throw RosettaErrorFactory.INTERNAL_SERVER_ERROR;
	}

	/**
	 * Retrieves (potentially cached) mosaic properties.
	 * @param {bigint} resolvedMosaicId Resolved mosaic id.
	 * @returns {object} Properties about the mosaic.
	 */
	async mosaicProperties(resolvedMosaicId) {
		if (this.mosaicPropertiesMap.has(resolvedMosaicId))
			return this.mosaicPropertiesMap.get(resolvedMosaicId);

		const results = await Promise.all([
			this.fetch(`mosaics/${bigIntToHexString(resolvedMosaicId)}`, jsonObject => jsonObject.mosaic.divisibility),
			this.fetch('namespaces/mosaic/names', jsonObject => jsonObject.mosaicNames[0], {
				method: 'POST',
				body: JSON.stringify({ mosaicIds: [bigIntToHexString(resolvedMosaicId)] }),
				headers: { 'Content-Type': 'application/json' }
			})
		]);

		const mosaic = {
			name: results[1].names.length ? results[1].names[0] : bigIntToHexString(resolvedMosaicId),
			divisibility: results[0]
		};
		this.mosaicPropertiesMap.set(resolvedMosaicId, mosaic);
		return mosaic;
	}
}
