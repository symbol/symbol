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

const aggregate = require('./aggregate/aggregate');
const empty = require('./empty');
const lockHash = require('./lockHash/lockHash');
const lockSecret = require('./lockSecret/lockSecret');
const metadata = require('./metadata/metadata');
const mosaic = require('./mosaic/mosaic');
const multisig = require('./multisig/multisig');
const namespace = require('./namespace/namespace');
const receipts = require('./receipts/receipts');
const restrictions = require('./restrictions/restrictions');
const MessageChannelBuilder = require('../connection/MessageChannelBuilder');

const plugins = {
	accountLink: empty,
	aggregate,
	lockHash,
	lockSecret,
	metadata,
	mosaic,
	multisig,
	namespace,
	receipts,
	restrictions,
	transfer: empty
};

module.exports = {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {array<string>} Names of all supported plugins.
	 */
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Configures the server with the specified extensions.
	 * @param {array} pluginNames Additional extensions to use.
	 * @param {object} server Server.
	 * @param {module:db/CatapultDb} db Catapult database.
	 * @param {object} services Supporting services.
	 * @returns {array<module:plugins/CatapultRestPlugin~TransactionStateDescriptor>} Additional transaction states to register.
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
