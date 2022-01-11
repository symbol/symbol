/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
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

/** @module utils/formattingUtils */

const formattingUtils = {
	/**
	 * Formats all the entities in an array.
	 * @param {module:utils/schemaFormatter~EntityFormatter} formatter Formatter.
	 * @param {Array} collection Array.
	 * @returns {Array} A new array of formatted entities.
	 */
	formatArray: (formatter, collection) => {
		const formattedEntities = [];
		collection.forEach(entity => { formattedEntities.push(formatter.format(entity)); });
		return formattedEntities;
	},

	/**
	 * Formats all the entities in a page.
	 * @param {module:utils/schemaFormatter~EntityFormatter} formatter Formatter.
	 * @param {object} collection Page collection containing the `data` array of results, and the `pagination` information object
	 * @returns {object} A new object of formatted page.
	 */
	formatPage: (formatter, collection) => {
		const formattedEntities = [];
		collection.data.forEach(entity => { formattedEntities.push(formatter.format(entity)); });
		return {
			data: formattedEntities,
			pagination: collection.pagination
		};
	}
};

module.exports = formattingUtils;
