import { createKeyPairFromPrivateKeyString, sign, verify } from './crypto/keyPair';
import sha3Hasher from './crypto/sha3Hasher';
import auth from './auth/auth';
import address from './model/address';
import EntityType from './model/EntityType';
import idReducer from './model/idReducer';
import ModelType from './model/ModelType';
import networkInfo from './model/networkInfo';
import serialize from './modelBinary/serialize';
import sizes from './modelBinary/sizes';
import packetHeader from './packet/header';
import PacketType from './packet/PacketType';
import BinaryParser from './parser/BinaryParser';
import PacketParser from './parser/PacketParser';
import catapultModelSystem from './plugins/catapultModelSystem';
import BinarySerializer from './serializer/BinarySerializer';
import SerializedSizeCalculator from './serializer/SerializedSizeCalculator';
import array from './utils/array';
import base32 from './utils/base32';
import convert from './utils/convert';
import formattingUtils from './utils/formattingUtils';
import future from './utils/future';
import objects from './utils/objects';
import schemaFormatter from './utils/schemaFormatter';
import SchemaType from './utils/SchemaType';
import uint64 from './utils/uint64';

export default {
	auth,
	constants: {
		sizes
	},
	crypto: {
		createKeyPairFromPrivateKeyString,
		sign,
		verify,
		sha3Hasher
	},
	model: {
		address,
		EntityType,
		idReducer,
		ModelType,
		networkInfo
	},
	modelBinary: {
		serialize
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
		array,
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
