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

const accountProperties = require('./accountProperties.js');
const aggregate = require('./aggregate');
const empty = require('./empty');
const lock = require('./lock');
const MessageChannelBuilder = require('../connection/MessageChannelBuilder');
const mosaic = require('./mosaic');
const multisig = require('./multisig');
const namespace = require('./namespace');

const plugins = {
	accountProperties, aggregate, lock, mosaic, multisig, namespace, transfer: empty
};

module.exports = {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {array<string>} The names of all supported plugins.
	 */
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Configures the server with the specified extensions.
	 * @param {array} pluginNames The additional extensions to use.
	 * @param {object} server The server.
	 * @param {module:db/CatapultDb} db The catapult database.
	 * @param {object} services Supporting services.
	 * @returns {array<module:plugins/CatapultRestPlugin~TransactionStateDescriptor>} The additional transaction states to register.
	 */
	configure: (pluginNames, server, db, services) => {
		const transactionStates = [];
		const messageChannelBuilder = new MessageChannelBuilder(services.config.websocket);
		(pluginNames || []).forEach(pluginName => {
			if (!plugins[pluginName])
				throw Error(`plugin '${pluginName}' not supported by route system`);

			const plugin = plugins[pluginName];
			plugin.registerTransactionStates(transactionStates);
			plugin.registerMessageChannels(messageChannelBuilder);
			plugin.registerRoutes(server, plugin.createDb(db), services);
		});

		return {
			transactionStates,
			messageChannelDescriptors: messageChannelBuilder.build()
		};
	}
};
