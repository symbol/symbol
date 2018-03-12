/** @module modelBinary/AggregateModelCodec */

// this file only contains an interface for prettier documentation, so ignore no-unused-vars warnings

/* eslint-disable no-unused-vars */

/**
 * Aggregate codec for serializing and deserializing a model supporting multiple entity types.
 * @interface
 * @extends {module:modelBinary/ModelCodec}
 */
module.exports = {
	/**
	 * Determines whether or not an entity type is supported.
	 * @instance
	 * @param {module:model/EntityType} type The entity type.
	 * @returns {boolean} true if the type is supported.
	 */
	supports: type => false
};

/* eslint-enable */
