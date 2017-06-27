import crypto from 'crypto';
import { createKeyPairFromPrivateKeyString } from '../src/crypto/keyPair';
import sizes from '../src/modelBinary/sizes';
import convert from '../src/utils/convert';

export default {
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
