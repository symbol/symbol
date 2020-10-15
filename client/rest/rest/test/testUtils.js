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

const catapult = require('catapult-sdk');
const MongoDb = require('mongodb');
const winston = require('winston');
const crypto = require('crypto');

const { sizes } = catapult.constants;
const random = {
	bytes: size => crypto.randomBytes(size),
	publicKey: () => crypto.randomBytes(sizes.signerPublicKey),
	hash: () => crypto.randomBytes(sizes.hash256),
	secret: () => crypto.randomBytes(sizes.hash256),
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
