/** @module plugins/namespace */
const mosaicRoutes = require('./routes/mosaicRoutes');
const namespaceRoutes = require('./routes/namespaceRoutes');
const NamespaceDb = require('./db/NamespaceDb');

/**
 * Creates a namespace plugin.
 * @type {module:plugins/CatapultRestPlugin}
 */
module.exports = {
	createDb: db => new NamespaceDb(db),

	registerTransactionStates: () => {},

	registerMessageChannels: () => {},

	registerRoutes: (...args) => {
		mosaicRoutes.register(...args);
		namespaceRoutes.register(...args);
	}
};
