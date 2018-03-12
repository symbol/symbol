/** @module utils/formattingUtils */

module.exports = {
	/**
	 * Formats all entities in an array.
	 * @param {module:utils/schemaFormatter~EntityFormatter} formatter The formatter.
	 * @param {Array} collection The array.
	 * @returns {Array} A new array of formatted entities.
	 */
	formatArray(formatter, collection) {
		const formattedEntities = [];
		collection.forEach(entity => { formattedEntities.push(formatter.format(entity)); });
		return formattedEntities;
	}
};
