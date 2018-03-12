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
