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

import aggregate from './aggregate/aggregate.js';
import empty from './empty.js';
import lockHash from './lockHash/lockHash.js';
import lockSecret from './lockSecret/lockSecret.js';
import metadata from './metadata/metadata.js';
import mosaic from './mosaic/mosaic.js';
import multisig from './multisig/multisig.js';
import namespace from './namespace/namespace.js';
import receipts from './receipts/receipts.js';
import restrictions from './restrictions/restrictions.js';
import nemRosetta from './rosetta/nem/rosetta.js';
import symbolRosetta from './rosetta/symbol/rosetta.js';
import MessageChannelBuilder from '../connection/MessageChannelBuilder.js';
import { NetworkLocator } from 'symbol-sdk';
import { Network } from 'symbol-sdk/symbol';

const plugins = {
	// transactions
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
	transfer: empty,

	// rosetta
	nemRosetta,
	symbolRosetta
};

export default {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {Array<string>} Names of all supported plugins.
	 */
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Configures the server with the specified extensions.
	 * @param {Array<string>} pluginNames Additional extensions to use.
	 * @param {object} server Server.
	 * @param {module:db/CatapultDb} db Catapult database.
	 * @param {object} services Supporting services.
	 * @returns {Array<module:plugins/CatapultRestPlugin~TransactionStateDescriptor>} Additional transaction states to register.
	 */
	configure: (pluginNames, server, db, services) => {
		const transactionStates = [];
		const networkIdentifier = NetworkLocator.findByName(Network.NETWORKS, services.config.network.name).identifier;
		const messageChannelBuilder = new MessageChannelBuilder(services.config.websocket, networkIdentifier);
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
