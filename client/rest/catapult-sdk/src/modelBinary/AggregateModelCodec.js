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

/** @module modelBinary/AggregateModelCodec */

// this file only contains an interface for prettier documentation, so ignore no-unused-vars warnings

/* eslint-disable no-unused-vars */

/**
 * Aggregate codec for serializing and deserializing a model supporting multiple entity types.
 * @interface
 * @extends {module:modelBinary/ModelCodec}
 */
const AggregateModelCodec = {
	/**
	 * Determines whether or not an entity type is supported.
	 * @instance
	 * @param {module:model/EntityType} type The entity type.
	 * @returns {boolean} true if the type is supported.
	 */
	supports: type => false
};

/* eslint-enable */
module.exports = AggregateModelCodec;
