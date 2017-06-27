/** @module utils/formattingUtils */

export default {
	/**
	 * Formats all entities in an array.
	 * @param {module:utils/schemaFormatter~EntityFormatter} formatter The formatter.
	 * @param {Array} collection The array.
	 * @returns {Array} A new array of formatted entities.
	 */
	formatArray(formatter, collection) {
		const formattedEntities = [];
		for (const entity of collection)
			formattedEntities.push(formatter.format(entity));

		return formattedEntities;
	}
};
