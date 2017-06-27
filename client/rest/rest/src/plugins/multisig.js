/** @module plugins/multisig */
import multisigRoutes from './routes/multisigRoutes';
import MultisigDb from './db/MultisigDb';

/**
 * Creates a multisig plugin.
 * @type {module:plugins/CatapultRestPlugin}
 */
export default {
	createDb: db => new MultisigDb(db),

	registerRoutes: (...args) => {
		multisigRoutes.register(...args);
	}
};
