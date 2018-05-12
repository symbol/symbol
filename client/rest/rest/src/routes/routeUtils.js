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

const catapult = require('catapult-sdk');
const errors = require('../server/errors');

const { address } = catapult.model;
const { convert } = catapult.utils;
const packetHeader = catapult.packet.header;
const constants = {
	sizes: {
		hexPublicKey: 64,
		addressEncoded: 40,
		hash256: 32,
		hash512: 64
	}
};

const isObjectId = str => 24 === str.length && convert.isHexString(str);

const namedParserMap = {
	objectId: str => {
		if (!isObjectId(str))
			throw Error('must be 12-byte hex string');

		return str;
	},
	uint: str => {
		const result = convert.tryParseUint(str);
		if (undefined === result)
			throw Error('must be non-negative number');

		return result;
	},
	address: str => {
		if (constants.sizes.addressEncoded === str.length)
			return address.stringToAddress(str);

		throw Error(`invalid length of address '${str.length}'`);
	},
	publicKey: str => {
		if (constants.sizes.hexPublicKey === str.length)
			return convert.hexToUint8(str);

		throw Error(`invalid length of publicKey '${str.length}'`);
	},
	accountId: str => {
		if (constants.sizes.hexPublicKey === str.length)
			return ['publicKey', convert.hexToUint8(str)];
		else if (constants.sizes.addressEncoded === str.length)
			return ['address', address.stringToAddress(str)];

		throw Error(`invalid length of account id '${str.length}'`);
	},
	hash256: str => {
		if (2 * constants.sizes.hash256 === str.length)
			return convert.hexToUint8(str);

		throw Error(`invalid length of hash256 '${str.length}'`);
	},
	hash512: str => {
		if (2 * constants.sizes.hash512 === str.length)
			return convert.hexToUint8(str);

		throw Error(`invalid length of hash512 '${str.length}'`);
	}
};

const routeUtils = {
	/**
	 * Parses an argument and throws an invalid argument error if it is invalid.
	 * @param {object} args The container containing the argument to parse.
	 * @param {string} key The name of the argument to parse.
	 * @param {Function|string} parser The parser to use or the name of a named parser.
	 * @returns {object} The parsed value.
	 */
	parseArgument: (args, key, parser) => {
		try {
			return ('string' === typeof parser ? namedParserMap[parser] : parser)(args[key]);
		} catch (err) {
			throw errors.createInvalidArgumentError(`${key} has an invalid format`, err);
		}
	},

	/**
	 * Parses an argument as an array and throws an invalid argument error if any element is invalid.
	 * @param {object} args The container containing the argument to parse.
	 * @param {string} key The name of the argument to parse.
	 * @param {Function|string} parser The parser to use or the name of a named parser.
	 * @returns {object} The array with parsed values.
	 */
	parseArgumentAsArray: (args, key, parser) => {
		const realParser = 'string' === typeof parser ? namedParserMap[parser] : parser;
		if (!Array.isArray(args[key]))
			throw errors.createInvalidArgumentError(`${key} has an invalid format: not an array`);

		try {
			return args[key].map(realParser);
		} catch (err) {
			throw errors.createInvalidArgumentError(`element in array ${key} has an invalid format`, err);
		}
	},

	/**
	 * Parses optional paging arguments and throws an invalid argument error if any is invalid.
	 * @param {object} args The arguments to parse.
	 * @returns {object} The parsed paging options.
	 */
	parsePagingArguments: args => {
		const parsedOptions = { id: undefined, pageSize: 0 };
		const parsers = {
			id: { tryParse: str => (isObjectId(str) ? str : undefined), type: 'object id' },
			pageSize: { tryParse: convert.tryParseUint, type: 'unsigned integer' }
		};

		Object.keys(parsedOptions).filter(key => args[key]).forEach(key => {
			const parser = parsers[key];
			parsedOptions[key] = parser.tryParse(args[key]);
			if (!parsedOptions[key])
				throw errors.createInvalidArgumentError(`${key} is not a valid ${parser.type}`);
		});

		return parsedOptions;
	},

	/**
	 * Generates valid page sizes from page size config.
	 * @param {object} config The page size config.
	 * @returns {object} The valid limits.
	 */
	generateValidPageSizes: config => {
		const pageSizes = [];
		const start = config.min + (0 === config.min % config.step ? 0 : config.step - (config.min % config.step));
		for (let pageSize = start; config.max >= pageSize; pageSize += config.step)
			pageSizes.push(pageSize);

		if (0 === pageSizes.length)
			throw Error('page size configuration does not specify any valid page sizes');

		return pageSizes;
	},

	/**
	 * Creates a sender for forwarding one or more objects of a given type.
	 * @param {module:routes/routeResultTypes} type The object type.
	 * @returns {object} The sender.
	 */
	createSender: type => ({
		/**
		 * Creates an array handler that forwards an array.
		 * @param {object} id The array identifier.
		 * @param {object} res The restify response object.
		 * @param {Function} next The restify next callback handler.
		 * @returns {Function} An appropriate array handler.
		 */
		sendArray(id, res, next) {
			return array => {
				if (!Array.isArray(array))
					res.send(errors.createInternalError(`error retrieving data for id: '${id}'`));
				else
					res.send({ payload: array, type });

				next();
			};
		},

		/**
		 * Creates an object handler that either forwards an object corresponding to an identifier
		 * or sends a not found error if no such object exists.
		 * @param {object} id The object identifier.
		 * @param {object} res The restify response object.
		 * @param {Function} next The restify next callback handler.
		 * @returns {Function} An appropriate object handler.
		 */
		sendOne(id, res, next) {
			const sendOneObject = object => {
				if (!object)
					res.send(errors.createNotFoundError(id));
				else
					res.send({ payload: object, type });
			};

			return object => {
				if (Array.isArray(object)) {
					if (2 <= object.length)
						res.send(errors.createInternalError(`error retrieving data for id: '${id}' (length ${object.length})`));
					else
						sendOneObject(object.length && object[0]);
				} else {
					sendOneObject(object);
				}

				next();
			};
		}
	}),

	/**
	 * Adds GET and POST routes for looking up documents of a single type.
	 * @param {object} server The server on which to register the routes.
	 * @param {object} sender The sender to use for sending the results.
	 * @param {object} routeInfo Information about the routes.
	 * @param {Function} documentRetriever Lookup function for retrieving the documents.
	 * @param {Function|string} parser The parser to use or the name of a named parser.
	 */
	addGetPostDocumentRoutes: (server, sender, routeInfo, documentRetriever, parser) => {
		const routes = {
			get: `${routeInfo.base}/:${routeInfo.singular}`,
			post: `${routeInfo.base}`
		};
		if (routeInfo.postfixes) {
			routes.get += `/${routeInfo.postfixes.singular}`;
			routes.post += `/${routeInfo.postfixes.plural}`;
		}

		server.get(routes.get, (req, res, next) => {
			const key = routeUtils.parseArgument(req.params, routeInfo.singular, parser);
			return documentRetriever([key]).then(sender.sendOne(req.params[routeInfo.singular], res, next));
		});

		server.post(routes.post, (req, res, next) => {
			const keys = routeUtils.parseArgumentAsArray(req.params, routeInfo.plural, parser);
			return documentRetriever(keys).then(sender.sendArray(req.params[routeInfo.plural], res, next));
		});
	},

	/**
	 * Adds PUT route for sending a packet to an api server.
 	 * @param {object} server The server on which to register the routes.
 	 * @param {object} connections The api server connection pool.
	 * @param {object} routeInfo Information about the route.
	 * @param {Function} parser The parser to use to parse the route parameters into a packet payload.
	 */
	addPutPacketRoute: (server, connections, routeInfo, parser) => {
		const createPacketFromBuffer = (data, packetType) => {
			const length = packetHeader.size + data.length;
			const header = packetHeader.createBuffer(packetType, length);
			const buffers = [header, Buffer.from(data)];
			return Buffer.concat(buffers, length);
		};

		server.put(routeInfo.routeName, (req, res, next) => {
			const packetBuffer = createPacketFromBuffer(parser(req.params), routeInfo.packetType);
			return connections.lease()
				.then(connection => connection.send(packetBuffer))
				.then(() => {
					res.send(202, { message: `packet ${routeInfo.packetType} was pushed to the network via ${routeInfo.routeName}` });
					next();
				});
		});
	}
};

module.exports = routeUtils;
