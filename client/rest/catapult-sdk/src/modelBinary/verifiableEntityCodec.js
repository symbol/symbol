/** @module modelBinary/verifiableEntityCodec */
const sizes = require('./sizes');

const constants = { sizes };

module.exports = {
	/**
	 * Parses a verifiable entity.
	 * @param {object} parser The parser.
	 * @returns {object} The parsed entity.
	 */
	deserialize: parser => {
		const entity = {};
		entity.signature = parser.buffer(constants.sizes.signature);
		entity.signer = parser.buffer(constants.sizes.signer);
		entity.version = parser.uint16();
		entity.type = parser.uint16();
		return entity;
	},

	/**
	 * Serializes a verifiable entity.
	 * @param {object} entity The entity.
	 * @param {object} serializer The serializer.
	 */
	serialize: (entity, serializer) => {
		serializer.writeBuffer(entity.signature);
		serializer.writeBuffer(entity.signer);
		serializer.writeUint16(entity.version);
		serializer.writeUint16(entity.type);
	}
};
