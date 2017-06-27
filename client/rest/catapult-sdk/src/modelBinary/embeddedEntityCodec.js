/** @module modelBinary/embeddedEntityCodec */
import sizes from './sizes';

const constants = { sizes };

export default {
	/**
	 * Parses an embedded entity.
	 * @param {object} parser The parser.
	 * @returns {object} The parsed entity.
	 */
	deserialize: parser => {
		const entity = {};
		entity.signer = parser.buffer(constants.sizes.signer);
		entity.version = parser.uint16();
		entity.type = parser.uint16();
		return entity;
	},

	/**
	 * Serializes an embedded entity.
	 * @param {object} entity The entity.
	 * @param {object} serializer The serializer.
	 */
	serialize: (entity, serializer) => {
		serializer.writeBuffer(entity.signer);
		serializer.writeUint16(entity.version);
		serializer.writeUint16(entity.type);
	}
};
