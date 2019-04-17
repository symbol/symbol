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

const address = require('./model/address');
const arrayUtils = require('./utils/arrayUtils');
const auth = require('./auth/auth');
const base32 = require('./utils/base32');
const BinaryParser = require('./parser/BinaryParser');
const BinarySerializer = require('./serializer/BinarySerializer');
const catapultModelSystem = require('./plugins/catapultModelSystem');
const convert = require('./utils/convert');
const EntityType = require('./model/EntityType');
const formattingUtils = require('./utils/formattingUtils');
const future = require('./utils/future');
const idReducer = require('./model/idReducer');
const merkle = require('./crypto/merkleAuditProof');
const ModelType = require('./model/ModelType');
const namespace = require('./model/namespace');
const networkInfo = require('./model/networkInfo');
const objects = require('./utils/objects');
const packetHeader = require('./packet/header');
const PacketParser = require('./parser/PacketParser');
const PacketType = require('./packet/PacketType');
const schemaFormatter = require('./utils/schemaFormatter');
const SchemaType = require('./utils/SchemaType');
const serialize = require('./modelBinary/serialize');
const SerializedSizeCalculator = require('./serializer/SerializedSizeCalculator');
const sha3Hasher = require('./crypto/sha3Hasher');
const sizes = require('./modelBinary/sizes');
const status = require('./model/status');
const transactionExtensions = require('./modelBinary/transactionExtensions');
const uint64 = require('./utils/uint64');
const { createKeyPairFromPrivateKeyString, sign, verify } = require('./crypto/keyPair');

const catapultSdk = {
	auth,
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
		PacketType
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
