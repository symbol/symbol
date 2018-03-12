/** @module plugins/lock */
const lockRoutes = require('./routes/lockRoutes');
const LockDb = require('./db/LockDb');

/**
 * Creates a lock plugin.
 * @type {module:plugins/CatapultRestPlugin}
 */
module.exports = {
	createDb: db => new LockDb(db),

	registerTransactionStates: () => {},

	registerMessageChannels: () => {},

	registerRoutes: (...args) => {
		lockRoutes.register(...args);
	}
};
