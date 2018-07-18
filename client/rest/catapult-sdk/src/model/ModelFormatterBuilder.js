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

/** @module model/ModelFormatterBuilder */
const schemaFormatter = require('../utils/schemaFormatter');

const createFormatter = (type, modelSchema, formattingRules) => ({
	format: entity => schemaFormatter.format(entity, modelSchema[type], modelSchema, formattingRules)
});

/**
 * Builder for creating a model formatter.
 */
class ModelFormatterBuilder {
	/**
	 * Creates a model formatter builder.
	 */
	constructor() {
		this.subFormatterTypes = new Set([
			'accountWithMetadata',
			'blockHeaderWithMetadata',
			'transactionWithMetadata',

			'chainInfo',
			'merkleProofInfo',
			'nodeInfo',
			'nodeTime',
			'storageInfo',
			'transactionStatus'
		]);
	}

	/**
	 * Adds support for a named formatter.
	 * @param {string} type The formatter type.
	 */
	addFormatter(type) {
		if (this.subFormatterTypes.has(type))
			throw Error(`formatter already registered for '${type}'`);

		this.subFormatterTypes.add(type);
	}

	/**
	 * Returns an appropriate aggregate formatter object.
	 * @param {object} modelSchema The model schema.
	 * @param {object} formattingRules A map for looking up formatting rules given a schema property type.
	 * @returns {object} The aggregate formatter object.
	 */
	build(modelSchema, formattingRules) {
		const formatter = {};
		this.subFormatterTypes.forEach(type => {
			formatter[type] = createFormatter(type, modelSchema, formattingRules);
		});

		return formatter;
	}
}

module.exports = ModelFormatterBuilder;
