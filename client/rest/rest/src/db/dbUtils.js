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

const errors = require('../server/errors');
const MongoDb = require('mongodb');

const { Long, ObjectId } = MongoDb;

const convertToLong = value => {
	if (Number.isInteger(value))
		return Long.fromNumber(value);

	// if value is an array, assume it is a uint64
	if (Array.isArray(value))
		return new Long(value[0], value[1]);

	if (value instanceof Long)
		return value;

	throw errors.createInvalidArgumentError(`${value} has an invalid format: not integer or uint64`);
};

const dbUtils = {
	/**
	 * Converts number to long.
	 * @param {object} value Value to convert.
	 * @returns {MongoDb.Long} Converted value.
	 */
	convertToLong,

	/**
	 * Converts long to uint64.
	 * @param {Long} value Value to convert.
	 * @returns {uint64} Converted value.
	 */
	longToUint64: value => {
		if (value instanceof Long)
			return [value.getLowBitsUnsigned(), value.getHighBits() >>> 0];

		throw errors.createInvalidArgumentError(`${value} has an invalid format: not long`);
	},

	/**
	 * Generates an offset condition depending on the offset type, and sorting options provided.
	 * @param {object} options Sorting options, must contain `offset`, `offsetType`, `sortField`, and `sortDirection`.
	 * @param {object} sortFieldDbRelation Determines the database path of the provided sort field.
	 * @returns {object} Offset condition if offset was provided, otherwise returns undefined.
	 */
	buildOffsetCondition: (options, sortFieldDbRelation) => {
		const offsetTypeToDbObject = {
			objectId: objectIdString => new ObjectId(objectIdString),
			uint64: convertToLong,
			uint64Hex: convertToLong
		};

		if (undefined !== options.offset) {
			const offsetRequiresParsing = Object.keys(offsetTypeToDbObject).includes(options.offsetType);
			const offset = offsetRequiresParsing ? offsetTypeToDbObject[options.offsetType](options.offset) : options.offset;
			return { [sortFieldDbRelation[options.sortField]]: { [1 === options.sortDirection ? '$gt' : '$lt']: offset } };
		}
		return undefined;
	}
};

module.exports = dbUtils;
