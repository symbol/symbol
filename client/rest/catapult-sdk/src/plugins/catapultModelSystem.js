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

/** @module plugins/catapultModelSystem */
const accountLink = require('./accountLink');
const accountProperties = require('./accountProperties');
const aggregate = require('./aggregate');
const lock = require('./lock');
const ModelCodecBuilder = require('../modelBinary/ModelCodecBuilder');
const ModelFormatterBuilder = require('../model/ModelFormatterBuilder');
const ModelSchemaBuilder = require('../model/ModelSchemaBuilder');
const mosaic = require('./mosaic');
const multisig = require('./multisig');
const namespace = require('./namespace');
const receipts = require('./receipts');
const transfer = require('./transfer');

const plugins = {
	accountLink, accountProperties, aggregate, lock, mosaic, multisig, namespace, receipts, transfer
};

/**
 * A complete catapult model system.
 * @class CatapultModelSystem
 *
 * @property {object} schema The complete schema information.
 */
const catapultModelSystem = {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {array<string>} The names of all supported plugins.
	 */
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Builds a catapult model system with the specified extensions.
	 * @param {array} pluginNames The additional extensions to use.
	 * @param {object} namedFormattingRules A dictionary containing named sets of formatting rules.
	 * @returns {module:plugins/catapultModelSystem} The configured catapult model system.
	 */
	configure: (pluginNames, namedFormattingRules) => {
		const schemaBuilder = new ModelSchemaBuilder();
		const codecBuilder = new ModelCodecBuilder();
		const formatterBuilder = new ModelFormatterBuilder();
		pluginNames.forEach(pluginName => {
			if (!plugins[pluginName])
				throw Error(`plugin '${pluginName}' not supported by model system`);

			const plugin = plugins[pluginName];
			plugin.registerSchema({
				addTransactionSupport: (transactionType, schema) => {
					schemaBuilder.addTransactionSupport(transactionType, schema);
					formatterBuilder.addFormatter(schemaBuilder.typeToName(transactionType));
				},
				addSchema: (name, schema) => {
					schemaBuilder.addSchema(name, schema);
					formatterBuilder.addFormatter(name);
				}
			});
			plugin.registerCodecs(codecBuilder);
		});

		const modelSchema = schemaBuilder.build();
		const formatters = {};
		Object.keys(namedFormattingRules).forEach(key => {
			formatters[key] = formatterBuilder.build(modelSchema, namedFormattingRules[key]);
		});

		return {
			schema: modelSchema,
			codec: codecBuilder.build(),
			formatters
		};
	}
};

module.exports = catapultModelSystem;
