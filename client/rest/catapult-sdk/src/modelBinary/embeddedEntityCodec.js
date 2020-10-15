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

/** @module modelBinary/embeddedEntityCodec */
const sizes = require('./sizes');

const constants = { sizes };

const embeddedEntityCodec = {
	/**
	 * Parses an embedded entity.
	 * @param {object} parser Parser.
	 * @returns {object} Parsed entity.
	 */
	deserialize: parser => {
		const entity = {};
		entity.embeddedTransactionHeader_Reserved1 = parser.uint32();
		entity.signerPublicKey = parser.buffer(constants.sizes.signerPublicKey);
		entity.entityBody_Reserved1 = parser.uint32();
		entity.version = parser.uint8();
		entity.network = parser.uint8();
		entity.type = parser.uint16();
		return entity;
	},

	/**
	 * Serializes an embedded entity.
	 * @param {object} entity Entity.
	 * @param {object} serializer Serializer.
	 */
	serialize: (entity, serializer) => {
		serializer.writeUint32(entity.embeddedTransactionHeader_Reserved1);
		serializer.writeBuffer(entity.signerPublicKey);
		serializer.writeUint32(entity.entityBody_Reserved1);
		serializer.writeUint8(entity.version);
		serializer.writeUint8(entity.network);
		serializer.writeUint16(entity.type);
	}
};

module.exports = embeddedEntityCodec;
