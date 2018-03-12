/** @module plugins/catapultModelSystem */
const aggregate = require('./aggregate');
const lock = require('./lock');
const multisig = require('./multisig');
const namespace = require('./namespace');
const transfer = require('./transfer');
const ModelFormatterBuilder = require('../model/ModelFormatterBuilder');
const ModelSchemaBuilder = require('../model/ModelSchemaBuilder');
const ModelCodecBuilder = require('../modelBinary/ModelCodecBuilder');

const plugins = {
	aggregate, lock, multisig, namespace, transfer
};

/**
 * A complete catapult model system.
 * @class CatapultModelSystem
 *
 * @property {object} schema The complete schema information.
 */
module.exports = {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {array<string>} The names of all supported plugins.
	 */
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Builds a catapult model system with the specified extensions.
	 * @param {array} pluginNames The additional extensions to use.
	 * @param {object} namedFormattingRules A dictionary containing named sets of formatting rules.
	 * @returns {module:plugins/catapultModelSystem} The configured catapult model system.
	 */
	configure: (pluginNames, namedFormattingRules) => {
		const schemaBuilder = new ModelSchemaBuilder();
		const codecBuilder = new ModelCodecBuilder();
		const formatterBuilder = new ModelFormatterBuilder();
		(pluginNames || []).forEach(pluginName => {
			if (!plugins[pluginName])
				throw Error(`plugin '${pluginName}' not supported by model system`);

			const plugin = plugins[pluginName];
			plugin.registerSchema({
				addTransactionSupport: (transactionType, schema) => {
					schemaBuilder.addTransactionSupport(transactionType, schema);
					formatterBuilder.addFormatter(schemaBuilder.typeToName(transactionType));
				},
				addSchema: (name, schema) => {
					schemaBuilder.addSchema(name, schema);
					formatterBuilder.addFormatter(name);
				}
			});
			plugin.registerCodecs(codecBuilder);
		});

		const modelSchema = schemaBuilder.build();
		const formatters = {};
		Object.keys(namedFormattingRules).forEach(key => {
			formatters[key] = formatterBuilder.build(modelSchema, namedFormattingRules[key]);
		});

		return {
			schema: modelSchema,
			codec: codecBuilder.build(),
			formatters
		};
	}
};
