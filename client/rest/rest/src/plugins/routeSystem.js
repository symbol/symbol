const aggregate = require('./aggregate');
const empty = require('./empty');
const lock = require('./lock');
const multisig = require('./multisig');
const namespace = require('./namespace');
const MessageChannelBuilder = require('../connection/MessageChannelBuilder');

const plugins = {
	aggregate, lock, multisig, namespace, transfer: empty
};

module.exports = {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {array<string>} The names of all supported plugins.
	 */
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Configures the server with the specified extensions.
	 * @param {array} pluginNames The additional extensions to use.
	 * @param {object} server The server.
	 * @param {module:db/CatapultDb} db The catapult database.
	 * @param {object} services Supporting services.
	 * @returns {array<module:plugins/CatapultRestPlugin~TransactionStateDescriptor>} The additional transaction states to register.
	 */
	configure: (pluginNames, server, db, services) => {
		const transactionStates = [];
		const messageChannelBuilder = new MessageChannelBuilder(services.config.websocket);
		(pluginNames || []).forEach(pluginName => {
			if (!plugins[pluginName])
				throw Error(`plugin '${pluginName}' not supported by route system`);

			const plugin = plugins[pluginName];
			plugin.registerTransactionStates(transactionStates);
			plugin.registerMessageChannels(messageChannelBuilder);
			plugin.registerRoutes(server, plugin.createDb(db), services);
		});

		return {
			transactionStates,
			messageChannelDescriptors: messageChannelBuilder.build()
		};
	}
};
