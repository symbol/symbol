import crypto from 'crypto';
import MongoDb from 'mongodb';
import winston from 'winston';

const Key_Size = 32;
const Hash_Size = 32;
const Signature_Size = 64;
const Address_Decoded_Size = 25;

export default {
	random: {
		bytes: size => crypto.randomBytes(size),
		publicKey: () => crypto.randomBytes(Key_Size),
		hash: () => crypto.randomBytes(Hash_Size),
		signature: () => crypto.randomBytes(Signature_Size),
		address: () => crypto.randomBytes(Address_Decoded_Size)
	},
	factory: {
		createBinary: buffer => new MongoDb.Binary(buffer),
		createLong: (low, high) => new MongoDb.Long(low, high),
		createObjectIdFromHexString: id => new MongoDb.ObjectID(id)
	},
	log: (...args) => {
		winston.debug(...args);
	}
};
