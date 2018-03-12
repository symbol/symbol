const catapult = require('catapult-sdk');
const crypto = require('crypto');
const MongoDb = require('mongodb');
const winston = require('winston');

const { sizes } = catapult.constants;
const random = {
	bytes: size => crypto.randomBytes(size),
	publicKey: () => crypto.randomBytes(sizes.signer),
	hash: () => crypto.randomBytes(sizes.hash),
	signature: () => crypto.randomBytes(sizes.signature),
	address: () => crypto.randomBytes(sizes.addressDecoded),
	account: () => ({
		publicKey: random.publicKey(),
		address: random.address()
	})
};

module.exports = {
	constants: { sizes },
	random,
	factory: {
		createBinary: buffer => new MongoDb.Binary(buffer),
		createLong: (low, high) => new MongoDb.Long(low, high),
		createObjectIdFromHexString: id => new MongoDb.ObjectID(id)
	},
	log: (...args) => {
		winston.debug(...args);
	},
	createLogger: () => winston,
	createMockLogger: () => {
		const logger = {};
		logger.numLogs = 0;
		['debug', 'info', 'warn', 'error'].forEach(level => {
			logger[level] = () => { ++logger.numLogs; };
		});
		return logger;
	}
};
