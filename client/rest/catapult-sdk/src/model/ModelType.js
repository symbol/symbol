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

const SchemaType = require('../utils/SchemaType');

/**
 * Catapult model extended schema property types.
 * @enum {numeric}
 * @extends module:utils/SchemaType
 * @exports model/ModelType
 */
const ModelType = {
	/** Schema property type indicating a binary value. */
	binary: SchemaType.max + 1,

	/** Schema property type indicating an object identifier. */
	objectId: SchemaType.max + 2,

	/** Schema property type indicating a status code. */
	statusCode: SchemaType.max + 3,

	/** Schema property type indicating a string value. */
	string: SchemaType.max + 4,

	/** Schema property type indicating an uint byte value. */
	uint8: SchemaType.max + 5,

	/** Schema property type indicating an uint16 value. */
	uint16: SchemaType.max + 6,

	/** Schema property type indicating an uint32 value. */
	uint32: SchemaType.max + 7,

	/** Schema property type indicating a uint64 value as string. */
	uint64: SchemaType.max + 8,

	/** Schema property type indicating a uint64 identifier as hexadecimal. */
	uint64HexIdentifier: SchemaType.max + 9,

	/** Schema property type indicating an int value. */
	int: SchemaType.max + 10,

	/** Schema property type indicating a boolean value. */
	boolean: SchemaType.max + 11
};

Object.assign(ModelType, SchemaType);
ModelType.max = ModelType.boolean;

module.exports = ModelType;
