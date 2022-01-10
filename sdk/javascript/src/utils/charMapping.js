/**
 * Builder for building a character map.
 * @class CharacterMapBuilder
 *
 * @property {object} map Character map.
 */

const charMapping = {
	/**
	 * Creates a builder for building a character map.
	 * @returns {module:utils/charMapping~CharacterMapBuilder} A character map builder.
	 */
	createBuilder: () => {
		const map = {};
		return {
			map,

			/**
			 * Adds a range mapping to the map.
			 * @param {string} start Start character.
			 * @param {string} end End character.
			 * @param {numeric} base Value corresponding to the start character.
			 * @memberof module:utils/charMapping~CharacterMapBuilder
			 * @instance
			 */
			addRange: (start, end, base) => {
				const startCode = start.charCodeAt(0);
				const endCode = end.charCodeAt(0);

				for (let code = startCode; code <= endCode; ++code)
					map[String.fromCharCode(code)] = code - startCode + base;
			}
		};
	}
};

module.exports = charMapping;
