/** @module plugins/CatapultRestPlugin */

// this file only contains an interface for prettier documentation, so ignore no-unused-vars warnings

/* eslint-disable no-unused-vars */

/**
 * Adds rest support for a particular subsystem.
 * @interface
 */
export default {
	/**
	 * Creates a plugin specific database.
	 * @instance
	 * @param {module:db/CatapultDb} db The catapult database.
	 */
	createDb: db => {},

	/**
	 * Registers route extensions.
	 * @instance
	 * @param {...args} args The arguments needed to register the routes.
	 */
	registerRoutes: (...args) => {}
};

/* eslint-enable */
