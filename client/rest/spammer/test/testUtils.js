const crypto = require('crypto');

module.exports = {
	random: {
		bytes: size => crypto.randomBytes(size)
	},

	buffer: {
		fromSize: size => {
			const buffer = Buffer.allocUnsafe(4);
			buffer.writeUInt32LE(size);
			return buffer;
		}
	}
};
