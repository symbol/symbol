/** @module plugins/catapultModelSystem */
import aggregate from './aggregate';
import multisig from './multisig';
import namespace from './namespace';
import transfer from './transfer';
import ModelFormatterBuilder from '../model/ModelFormatterBuilder';
import ModelSchemaBuilder from '../model/ModelSchemaBuilder';
import ModelCodecBuilder from '../modelBinary/ModelCodecBuilder';

const plugins = { aggregate, multisig, namespace, transfer };

/**
 * A complete catapult model system.
 * @class CatapultModelSystem
 *
 * @property {object} schema The complete schema information.
 */
export default {
	/**
	 * Gets the names of all supported plugins.
	 * @returns {array<string>} The names of all supported plugins.
	 **/
	supportedPluginNames: () => Object.keys(plugins),

	/**
	 * Builds a catapult model system with the specified extensions.
	 * @param {array} pluginNames The additional extensions to use.
	 * @param {object} formattingRules The formatting rules to use.
	 * @returns {module:plugins/catapultModelSystem} The configured catapult model system.
	 */
	configure: (pluginNames, formattingRules) => {
		const schemaBuilder = new ModelSchemaBuilder();
		const codecBuilder = new ModelCodecBuilder();
		const formatterBuilder = new ModelFormatterBuilder();
		for (const pluginName of pluginNames || []) {
			if (!plugins[pluginName])
				throw Error(`plugin '${pluginName}' not supported by model system`);

			const plugin = plugins[pluginName];
			plugin.registerSchema({
				addTransactionSupport: (name, schema) => {
					schemaBuilder.addTransactionSupport(name, schema);
					formatterBuilder.addFormatter(name);
				},
				addSchema: (name, schema) => {
					schemaBuilder.addSchema(name, schema);
					formatterBuilder.addFormatter(name);
				}
			});
			plugin.registerCodecs(codecBuilder);
		}

		const modelSchema = schemaBuilder.build();
		return {
			schema: modelSchema,
			codec: codecBuilder.build(),
			formatter: formatterBuilder.build(modelSchema, formattingRules)
		};
	}
};
