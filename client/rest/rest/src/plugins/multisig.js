/** @module plugins/multisig */
const multisigRoutes = require('./routes/multisigRoutes');
const MultisigDb = require('./db/MultisigDb');

/**
 * Creates a multisig plugin.
 * @type {module:plugins/CatapultRestPlugin}
 */
module.exports = {
	createDb: db => new MultisigDb(db),

	registerTransactionStates: () => {},

	registerMessageChannels: () => {},

	registerRoutes: (...args) => {
		multisigRoutes.register(...args);
	}
};
