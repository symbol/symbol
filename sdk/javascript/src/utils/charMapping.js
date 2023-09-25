/**
 * Builder for building a character map.
 */
class CharacterMapBuilder {
	/**
	 * Creates a new character map builder.
	 */
	constructor() {
		/**
		 * Mapping of characters to character codes.
		 * @type {{[key: string]: number}}
		 */
		this.map = {};
	}

	/**
	 * Adds a range mapping to the map.
	 * @param {string} start Start character.
	 * @param {string} end End character.
	 * @param {number} base Value corresponding to the start character.
	 */
	addRange(start, end, base) {
		const startCode = start.charCodeAt(0);
		const endCode = end.charCodeAt(0);

		for (let code = startCode; code <= endCode; ++code)
			this.map[String.fromCharCode(code)] = code - startCode + base;
	}
}

const charMapping = {
	/**
	 * Creates a builder for building a character map.
	 * @returns {CharacterMapBuilder} Character map builder.
	 */
	createBuilder: () => new CharacterMapBuilder()
};

export default charMapping;
