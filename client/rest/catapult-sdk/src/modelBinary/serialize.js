/** @module modelBinary/serialize */
const BinarySerializer = require('../serializer/BinarySerializer');
const SerializedSizeCalculator = require('../serializer/SerializedSizeCalculator');
const convert = require('../utils/convert');

const serializeToBuffer = (codec, entity) => {
	const calculator = new SerializedSizeCalculator();
	codec.serialize(entity, calculator);

	const serializer = new BinarySerializer(calculator.size());
	codec.serialize(entity, serializer);
	return serializer.buffer();
};

/**
 * Serializer utility functions.
 */
module.exports = {
	/**
	 * Serializes an entity to a hex string using a codec.
	 * @param {module:modelBinary/ModelCodec} codec The model codec.
	 * @param {object} entity The entity to serialize.
	 * @returns {string} A hex string representing the entity.
	 */
	toHex: (codec, entity) => convert.uint8ToHex(serializeToBuffer(codec, entity)),

	/**
	 * Serializes an entity to a buffer using a codec.
	 * @param {module:modelBinary/ModelCodec} codec The model codec.
	 * @param {object} entity The entity to serialize.
	 * @returns {Buffer} A buffer representing the entity.
	 */
	toBuffer: serializeToBuffer
};
