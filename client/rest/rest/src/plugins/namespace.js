/** @module plugins/namespace */
import mosaicRoutes from './routes/mosaicRoutes';
import namespaceRoutes from './routes/namespaceRoutes';
import NamespaceDb from './db/NamespaceDb';

/**
 * Creates a namespace plugin.
 * @type {module:plugins/CatapultRestPlugin}
 */
export default {
	createDb: db => new NamespaceDb(db),

	registerRoutes: (...args) => {
		mosaicRoutes.register(...args);
		namespaceRoutes.register(...args);
	}
};
