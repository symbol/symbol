/** @module modelBinary/ModelCodec */

// this file only contains an interface for prettier documentation, so ignore no-unused-vars warnings

/* eslint-disable no-unused-vars */

/**
 * Codec for serializing and deserializing a model.
 * @interface
 */
export default {
	/**
	 * Deserializes a model.
	 * @instance
	 * @param {object} parser The parser.
	 * @returns {object} The parsed model.
	 */
	deserialize: parser => undefined,

	/**
	 * Serializes a model.
	 * @instance
	 * @param {object} entity The model.
	 * @param {object} serializer The serializer.
	 */
	serialize: (entity, serializer) => {}
};

/* eslint-enable */
