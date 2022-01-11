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

/** @module utils/SchemaType */

/**
 * Basic schema property types.
 * @enum {numeric}
 */
const SchemaType = {
	/** Default schema property type. */
	none: 0,

	/** Schema property type indicating an array. */
	array: 1,

	/** Schema property type indicating a dictionary. */
	dictionary: 2,

	/** Schema property type indicating an object. */
	object: 3,

	/** Maximum value in this enumeration. */
	max: 3
};

module.exports = SchemaType;
