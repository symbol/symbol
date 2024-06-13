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

/** @module plugins/catapultModelSystem */
import accountLink from './accountLink.js';
import aggregate from './aggregate.js';
import lockHash from './lockHash.js';
import lockSecret from './lockSecret.js';
import metadata from './metadata.js';
import mosaic from './mosaic.js';
import multisig from './multisig.js';
import namespace from './namespace.js';
import receipts from './receipts.js';
import restrictions from './restrictions.js';
import transfer from './transfer.js';
import ModelFormatterBuilder from '../model/ModelFormatterBuilder.js';
import ModelSchemaBuilder from '../model/ModelSchemaBuilder.js';

const plugins = {
	accountLink,
	aggregate,
	lockHash,
	lockSecret,
	metadata,
	mosaic,
	multisig,
	namespace,
	receipts,
	restrictions,
	transfer
};

/**
 * A complete catapult model system.
 * @class CatapultModelSystem
 * @property {object} schema Complete schema information.
 */
export default {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {Array<string>} Names of all supported plugins.
	 */
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Builds a catapult model system with the specified extensions.
	 * @param {Array<object>} pluginNames Additional extensions to use.
	 * @param {object} namedFormattingRules A dictionary containing named sets of formatting rules.
	 * @returns {module:plugins/catapultModelSystem} Configured catapult model system.
	 */
	configure: (pluginNames, namedFormattingRules) => {
		const schemaBuilder = new ModelSchemaBuilder();
		const formatterBuilder = new ModelFormatterBuilder();
		pluginNames.forEach(pluginName => {
			if (!plugins[pluginName])
				throw Error(`plugin '${pluginName}' not supported by model system`);

			const plugin = plugins[pluginName];
			plugin.registerSchema({
				addTransactionSupport: (transactionType, schema) => {
					schemaBuilder.addTransactionSupport(transactionType, schema);
					formatterBuilder.addFormatter(transactionType.toString());
				},
				addSchema: (name, schema) => {
					schemaBuilder.addSchema(name, schema);
					formatterBuilder.addFormatter(name);
				}
			});
		});

		const modelSchema = schemaBuilder.build();
		const formatters = {};
		Object.keys(namedFormattingRules).forEach(key => {
			formatters[key] = formatterBuilder.build(modelSchema, namedFormattingRules[key]);
		});

		return {
			schema: modelSchema,
			formatters
		};
	}
};
