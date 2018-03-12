/** @module utils/SchemaType */

/**
 * Basic schema property types.
 * @enum {numeric}
 */
module.exports = {
	/** The default schema property type. */
	none: 0,

	/** Schema property type indicating an object. */
	object: 1,

	/** Schema property type indicating an array. */
	array: 2,

	/** Schema property type indicating a dictionary. */
	dictionary: 3,

	/** The maximum value in this enumeration. */
	max: 3
};
