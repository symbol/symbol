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

/** @module modelBinary/serialize */
const BinarySerializer = require('../serializer/BinarySerializer');
const convert = require('../utils/convert');
const SerializedSizeCalculator = require('../serializer/SerializedSizeCalculator');

const serializeToBuffer = (codec, entity) => {
	const calculator = new SerializedSizeCalculator();
	codec.serialize(entity, calculator);

	const serializer = new BinarySerializer(calculator.size());
	codec.serialize(entity, serializer);
	return serializer.buffer();
};

/**
 * Serializer utility functions.
 */
const serialize = {
	/**
	 * Serializes an entity to a hex string using a codec.
	 * @param {module:modelBinary/ModelCodec} codec The model codec.
	 * @param {object} entity The entity to serialize.
	 * @returns {string} A hex string representing the entity.
	 */
	toHex: (codec, entity) => convert.uint8ToHex(serializeToBuffer(codec, entity)),

	/**
	 * Serializes an entity to a buffer using a codec.
	 * @param {module:modelBinary/ModelCodec} codec The model codec.
	 * @param {object} entity The entity to serialize.
	 * @returns {Buffer} A buffer representing the entity.
	 */
	toBuffer: serializeToBuffer
};

module.exports = serialize;
