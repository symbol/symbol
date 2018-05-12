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

/** @module plugins/CatapultRestPlugin */

// this file only contains an interface for prettier documentation, so ignore no-unused-vars warnings

/* eslint-disable no-unused-vars */

/**
 * A transaction state descriptor.
 * @typedef {object} TransactionStateDescriptor
 * @property {string} friendlyName The friendly name.
 * @property {string} dbPostfix The database function name postfix.
 * @property {string} routePostfix The route postfix.
 */

/**
 * Adds rest support for a particular subsystem.
 * @interface
 */
module.exports = {
	/**
	 * Creates a plugin specific database.
	 * @instance
	 * @param {module:db/CatapultDb} db The catapult database.
	 */
	createDb: db => {},

	/**
	 * Registers transaction state descriptors.
	 * @instance
	 * @param {array<module:plugins/CatapultRestPlugin~TransactionStateDescriptor>} states The transaction state descriptors.
	 */
	registerTransactionStates: states => {},

	/**
	 * Registers message channels.
	 * @instance
	 * @param {module:connection/MessageChannelBuilder~MessageChannelBuilder} builder The message channel builder.
	 */
	registerMessageChannels: builder => {},

	/**
	 * Registers route extensions.
	 * @instance
	 * @param {...args} args The arguments needed to register the routes.
	 */
	registerRoutes: (...args) => {}
};

/* eslint-enable */
