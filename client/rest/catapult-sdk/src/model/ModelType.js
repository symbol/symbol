import SchemaType from '../utils/SchemaType';

/**
 * Catapult model extended schema property types.
 * @enum {numeric}
 * @extends module:utils/SchemaType
 * @exports model/ModelType
 */
const ModelType = {
	/** Schema property type indicating a binary value. */
	binary: SchemaType.max + 1,

	/** Schema property type indicating a uint64 value. */
	uint64: SchemaType.max + 2,

	/** Schema property type indicating an object identifier. */
	objectId: SchemaType.max + 3,

	/** Schema property type indicating a string value. */
	string: SchemaType.max + 4
};

Object.assign(ModelType, SchemaType);
ModelType.max = ModelType.string;

export default ModelType;
