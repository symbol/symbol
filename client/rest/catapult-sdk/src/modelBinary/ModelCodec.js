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

/** @module modelBinary/ModelCodec */

// this file only contains an interface for prettier documentation, so ignore no-unused-vars warnings

/* eslint-disable no-unused-vars */

/**
 * Codec for serializing and deserializing a model.
 * @interface
 */
const ModelCodec = {
	/**
	 * Deserializes a model.
	 * @instance
	 * @param {object} parser The parser.
	 * @param {object} options Optional implementation-dependent deserialization options.
	 * @returns {object} The parsed model.
	 */
	deserialize: (parser, options) => undefined,

	/**
	 * Serializes a model.
	 * @instance
	 * @param {object} entity The model.
	 * @param {object} serializer The serializer.
	 */
	serialize: (entity, serializer) => {}
};

/* eslint-enable */
module.exports = ModelCodec;
