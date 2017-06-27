import empty from './empty';
import multisig from './multisig';
import namespace from './namespace';

const plugins = { aggregate: empty, multisig, namespace, transfer: empty };

export default {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {array<string>} The names of all supported plugins.
	 **/
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Configures the server with the specified extensions.
	 * @param {array} pluginNames The additional extensions to use.
	 * @param {object} server The server.
	 * @param {module:db/CatapultDb} db The catapult database.
	 */
	configure: (pluginNames, server, db) => {
		for (const pluginName of pluginNames || []) {
			if (!plugins[pluginName])
				throw Error(`plugin '${pluginName}' not supported by route system`);

			const plugin = plugins[pluginName];
			plugin.registerRoutes(server, plugin.createDb(db));
		}
	}
};
