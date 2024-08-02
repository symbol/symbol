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

import { RosettaErrorFactory } from '../rosettaUtils.js';

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
}
