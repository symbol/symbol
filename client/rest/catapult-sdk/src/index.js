const { createKeyPairFromPrivateKeyString, sign, verify } = require('./crypto/keyPair');
const sha3Hasher = require('./crypto/sha3Hasher');
const auth = require('./auth/auth');
const address = require('./model/address');
const EntityType = require('./model/EntityType');
const idReducer = require('./model/idReducer');
const ModelType = require('./model/ModelType');
const networkInfo = require('./model/networkInfo');
const status = require('./model/status');
const serialize = require('./modelBinary/serialize');
const sizes = require('./modelBinary/sizes');
const transactionExtensions = require('./modelBinary/transactionExtensions');
const packetHeader = require('./packet/header');
const PacketType = require('./packet/PacketType');
const BinaryParser = require('./parser/BinaryParser');
const PacketParser = require('./parser/PacketParser');
const catapultModelSystem = require('./plugins/catapultModelSystem');
const BinarySerializer = require('./serializer/BinarySerializer');
const SerializedSizeCalculator = require('./serializer/SerializedSizeCalculator');
const array = require('./utils/array');
const base32 = require('./utils/base32');
const convert = require('./utils/convert');
const formattingUtils = require('./utils/formattingUtils');
const future = require('./utils/future');
const objects = require('./utils/objects');
const schemaFormatter = require('./utils/schemaFormatter');
const SchemaType = require('./utils/SchemaType');
const uint64 = require('./utils/uint64');

module.exports = {
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
