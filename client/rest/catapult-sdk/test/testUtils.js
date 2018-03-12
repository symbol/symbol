const crypto = require('crypto');
const { createKeyPairFromPrivateKeyString } = require('../src/crypto/keyPair');
const sizes = require('../src/modelBinary/sizes');
const convert = require('../src/utils/convert');

module.exports = {
	constants: { sizes },

	random: {
		bytes: size => crypto.randomBytes(size),
		publicKey: () => crypto.randomBytes(sizes.signer),
		keyPair: () => createKeyPairFromPrivateKeyString(convert.uint8ToHex(crypto.randomBytes(sizes.signer)))
	},

	buffer: {
		fromSize: size => {
			const buffer = Buffer.allocUnsafe(4);
			buffer.writeUInt32LE(size);
			return buffer;
		}
	}
};
