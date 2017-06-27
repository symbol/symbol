/** @module plugins/CatapultPlugin */

// this file only contains an interface for prettier documentation, so ignore no-unused-vars warnings

/* eslint-disable no-unused-vars */

/**
 * Adds support for a particular subsystem.
 * @interface
 */
export default {
	/**
	 * Registers schema extensions.
	 * @instance
	 * @param {module:model/ModelSchemaBuilder} schemaBuilder The schema builder to augment.
	 */
	registerSchema: schemaBuilder => {},

	/**
	 * Registers codecs for serializing and deserializing transactions.
	 * @instance
	 * @param {module:modelBinary/ModelCodecBuilder} codecBuilder The codec builder to augment.
	 */
	registerCodecs: codecBuilder => {}
};

/* eslint-enable */
