/** @module plugins/empty */

/**
 * Creates an empty plugin.
 * @type {module:plugins/CatapultRestPlugin}
 */
module.exports = {
	createDb: () => undefined,

	registerTransactionStates: () => {},

	registerMessageChannels: () => {},

	registerRoutes: () => {}
};
