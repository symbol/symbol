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

const dbFacade = require('./dbFacade');
const routeResultTypes = require('./routeResultTypes');
const errors = require('../server/errors');
const catapult = require('catapult-sdk');

const { address } = catapult.model;
const { buildAuditPath, indexOfLeafWithHash } = catapult.crypto.merkle;
const { convert, uint64 } = catapult.utils;
const packetHeader = catapult.packet.header;
const constants = {
	sizes: {
		hexPublicKey: 64,
		addressEncoded: 39,
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
	uint64: str => uint64.fromString(str),
	uint64hex: str => uint64.fromHex(str),
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
		if (constants.sizes.addressEncoded === str.length)
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
	},
	boolean: str => {
		if (('true' !== str) && ('false' !== str))
			throw Error('must be boolean value \'true\' or \'false\'');

		return 'true' === str;
	}
};

const getBoundedPageSize = (pageSize, optionsPageSize) =>
	Math.max(optionsPageSize.min, Math.min(optionsPageSize.max, pageSize || optionsPageSize.default));

const isPage = page => undefined !== page.data && undefined !== page.pagination.pageNumber && undefined !== page.pagination.pageSize;

const routeUtils = {
	/**
	 * Parses an argument and throws an invalid argument error if it is invalid.
	 * @param {object} args Container containing the argument to parse.
	 * @param {string} key Name of the argument to parse.
	 * @param {Function|string} parser Parser to use or the name of a named parser.
	 * @returns {object} Parsed value.
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
	 * @param {object} args Container containing the argument to parse.
	 * @param {string} key Name of the argument to parse.
	 * @param {Function|string} parser Parser to use or the name of a named parser.
	 * @returns {object} Array with parsed values.
	 */
	parseArgumentAsArray: (args, key, parser) => {
		const realParser = 'string' === typeof parser ? namedParserMap[parser] : parser;
		let providedArgs = args[key];
		if (!Array.isArray(providedArgs))
			providedArgs = [providedArgs];

		try {
			return providedArgs.map(realParser);
		} catch (err) {
			throw errors.createInvalidArgumentError(`element in array ${key} has an invalid format`, err);
		}
	},

	/**
	 * Parses pagination arguments and throws an invalid argument error if any is invalid.
	 * @param {object} args Arguments to parse.
	 * @param {object} optionsPageSize Page size options.
	 * @param {object} offsetParsers Sort fields with the related offset parser this endpoint allows, will match provided `sortField` and
	 * throw if invalid. Must have at least one entry, and `id` is treated as default if no `sortField` is provided.
	 * @returns {object} Parsed pagination options.
	 */
	parsePaginationArguments: (args, optionsPageSize, offsetParsers) => {
		const allowedSortFields = Object.keys(offsetParsers);
		if (args.orderBy && !allowedSortFields.includes(args.orderBy))
			throw errors.createInvalidArgumentError(`sorting by ${args.orderBy} is not allowed`);

		const parsedArgs = {
			sortField: allowedSortFields.includes(args.orderBy) ? args.orderBy : 'id',
			sortDirection: 'desc' === args.order ? -1 : 1
		};

		if (args.pageSize) {
			const numericPageSize = convert.tryParseUint(args.pageSize);
			if (undefined === numericPageSize)
				throw errors.createInvalidArgumentError('pageSize is not a valid unsigned integer');

			parsedArgs.pageSize = getBoundedPageSize(numericPageSize, optionsPageSize);
		} else {
			parsedArgs.pageSize = optionsPageSize.default;
		}

		if (args.pageNumber) {
			const numericPageNumber = convert.tryParseUint(args.pageNumber);
			if (undefined === numericPageNumber)
				throw errors.createInvalidArgumentError('pageNumber is not a valid unsigned integer');

			parsedArgs.pageNumber = numericPageNumber;
		}
		parsedArgs.pageNumber = 0 < parsedArgs.pageNumber ? parsedArgs.pageNumber : 1;

		if (args.offset) {
			parsedArgs.offset = routeUtils.parseArgument(args, 'offset', offsetParsers[parsedArgs.sortField]);
			parsedArgs.offsetType = offsetParsers[parsedArgs.sortField];
		}

		return parsedArgs;
	},

	/**
	 * Creates a sender for forwarding one or more objects of a given type.
	 * @param {module:routes/routeResultTypes} type Object type.
	 * @returns {object} Sender.
	 */
	createSender: type => ({
		/**
		 * Creates an array handler that forwards an array.
		 * @param {object} id Array identifier.
		 * @param {object} res Restify response object.
		 * @param {Function} next Restify next callback handler.
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
		 * @param {object} id Object identifier.
		 * @param {object} res Restify response object.
		 * @param {Function} next Restify next callback handler.
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
		},

		/**
		 * Creates a page handler that forwards a paginated result.
		 * @param {object} res Restify response object.
		 * @param {Function} next Restify next callback handler.
		 * @returns {Function} An appropriate object handler.
		 */
		sendPage(res, next) {
			return page => {
				if (!isPage(page))
					res.send(errors.createInternalError('error retrieving data'));
				else
					res.send({ payload: page, type, structure: 'page' });
				next();
			};
		}
	}),

	/**
	 * Adds GET and POST routes for looking up documents of a single type.
	 * @param {object} server Server on which to register the routes.
	 * @param {object} sender Sender to use for sending the results.
	 * @param {object} routeInfo Information about the routes.
	 * @param {Function} documentRetriever Lookup function for retrieving the documents.
	 * @param {Function|string} parser Parser to use or the name of a named parser.
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
 	 * @param {object} server Server on which to register the routes.
 	 * @param {object} connections Api server connection pool.
	 * @param {object} routeInfo Information about the route.
	 * @param {Function} parser Parser to use to parse the route parameters into a packet payload.
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
	},

	/**
	 * Returns function for processing merkle tree path requests.
	 * @param {module:db/CatapultDb} db Catapult database.
	 * @param {string} blockMetaCountField Field name for block meta count.
	 * @param {string} blockMetaTreeField Field name for block meta merkle tree.
	 * @returns {Function} Restify response function to process merkle path requests.
	 */
	blockRouteMerkleProcessor: (db, blockMetaCountField, blockMetaTreeField) => (req, res, next) => {
		const height = routeUtils.parseArgument(req.params, 'height', 'uint64');
		const hash = routeUtils.parseArgument(req.params, 'hash', 'hash256');

		return dbFacade.runHeightDependentOperation(db, height, () => db.blockWithMerkleTreeAtHeight(height, blockMetaTreeField))
			.then(result => {
				if (!result.isRequestValid) {
					res.send(errors.createNotFoundError(uint64.toString(height)));
					return next();
				}

				const block = result.payload;
				if (!block.meta[blockMetaCountField]) {
					res.send(errors.createInvalidArgumentError(
						`hash '${req.params.hash}' not included in block height '${uint64.toString(height)}'`
					));
					return next();
				}

				const merkleTree = {
					count: block.meta[blockMetaCountField],
					nodes: block.meta[blockMetaTreeField].map(merkleHash => merkleHash.buffer)
				};

				if (0 > indexOfLeafWithHash(hash, merkleTree)) {
					res.send(errors.createInvalidArgumentError(
						`hash '${req.params.hash}' not included in block height '${uint64.toString(height)}'`
					));
					return next();
				}

				const merklePath = buildAuditPath(hash, merkleTree);

				res.send({
					payload: { merklePath },
					type: routeResultTypes.merkleProofInfo
				});

				return next();
			});
	},

	/**
	 * Returns account public key from account address .
	 * @param {module:db/CatapultDb} db Catapult database.
	 * @param {Uint8Array} accountAddress Account address.
	 * @returns {Promise<Uint8Array>} Account public key.
	 */
	addressToPublicKey: (db, accountAddress) => db.addressToPublicKey(accountAddress)
		.then(result => {
			if (!result)
				return Promise.reject(Error('account not found'));

			return result.account.publicKey.buffer;
		})
};

module.exports = routeUtils;
