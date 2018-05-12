/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @module utils/charMapping */

/**
 * Builder for building a character map.
 * @class CharacterMapBuilder
 *
 * @property {object} map The character map.
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
			 * @param {string} start The start character.
			 * @param {string} end The end character.
			 * @param {numeric} base The value corresponding to the start character.
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
