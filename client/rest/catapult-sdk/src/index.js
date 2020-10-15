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

const { createKeyPairFromPrivateKeyString, sign, verify } = require('./crypto/keyPair');
const merkle = require('./crypto/merkleAuditProof');
const sha3Hasher = require('./crypto/sha3Hasher');
const EntityType = require('./model/EntityType');
const ModelType = require('./model/ModelType');
const address = require('./model/address');
const idReducer = require('./model/idReducer');
const namespace = require('./model/namespace');
const networkInfo = require('./model/networkInfo');
const restriction = require('./model/restriction');
const status = require('./model/status');
const serialize = require('./modelBinary/serialize');
const sizes = require('./modelBinary/sizes');
const transactionExtensions = require('./modelBinary/transactionExtensions');
const { PacketType, StatePathPacketTypes } = require('./packet/PacketType');
const packetHeader = require('./packet/header');
const BinaryParser = require('./parser/BinaryParser');
const PacketParser = require('./parser/PacketParser');
const catapultModelSystem = require('./plugins/catapultModelSystem');
const BinarySerializer = require('./serializer/BinarySerializer');
const SerializedSizeCalculator = require('./serializer/SerializedSizeCalculator');
const SchemaType = require('./utils/SchemaType');
const arrayUtils = require('./utils/arrayUtils');
const base32 = require('./utils/base32');
const convert = require('./utils/convert');
const formattingUtils = require('./utils/formattingUtils');
const future = require('./utils/future');
const objects = require('./utils/objects');
const schemaFormatter = require('./utils/schemaFormatter');
const uint64 = require('./utils/uint64');

const catapultSdk = {
	constants: {
		sizes
	},
	crypto: {
		createKeyPairFromPrivateKeyString,
		merkle,
		sha3Hasher,
		sign,
		verify
	},
	model: {
		address,
		EntityType,
		idReducer,
		ModelType,
		restriction,
		namespace,
		networkInfo,
		status
	},
	modelBinary: {
		serialize,
		transactionExtensions
	},
	packet: {
		header: packetHeader,
		PacketType,
		StatePathPacketTypes
	},
	parser: {
		BinaryParser,
		PacketParser
	},
	plugins: {
		catapultModelSystem
	},
	serializer: {
		BinarySerializer,
		SerializedSizeCalculator
	},
	utils: {
		array: arrayUtils,
		base32,
		convert,
		formattingUtils,
		future,
		objects,
		schemaFormatter,
		SchemaType,
		uint64
	}
};

module.exports = catapultSdk;
