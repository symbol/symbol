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

import dbFacade from './dbFacade.js';
import routeResultTypes from './routeResultTypes.js';
import catapult from '../catapult-sdk/index.js';
import errors from '../server/errors.js';
import { utils } from 'symbol-sdk';
import { Address } from 'symbol-sdk/symbol';

const { buildAuditPath, indexOfLeafWithHash } = catapult.crypto.merkle;
const packetHeader = catapult.packet.header;
const constants = {
	sizes: {
		hexPublicKey: 64,
		addressEncoded: 39,
		addressDecoded: 24,
		hash256: 32,
		hash512: 64
	}
};

const isObjectId = str => 24 === str.length && utils.isHexString(str);

const namedParserMap = {
	objectId: str => {
		if (!isObjectId(str))
			throw Error('must be 12-byte hex string');

		return str;
	},
	uint: str => {
		const result = utils.tryParseUint(str);
		if (undefined === result)
			throw Error('must be non-negative number');

		return result;
	},
	uint64: str => {
		const value = BigInt(str);
		if (0n > value)
			throw Error('must be non-negative');

		return value;
	},
	uint64hex: str => {
		if (16 !== str.length)
			throw Error('must be 8 hex digits in length');

		return BigInt(`0x${str}`);
	},
	address: str => {
		if (constants.sizes.addressEncoded === str.length)
			return new Address(str).bytes;

		throw Error(`invalid length of address '${str.length}'`);
	},
	publicKey: str => {
		if (constants.sizes.hexPublicKey === str.length)
			return utils.hexToUint8(str);

		throw Error(`invalid length of publicKey '${str.length}'`);
	},
	accountId: str => {
		if (constants.sizes.hexPublicKey === str.length)
			return ['publicKey', utils.hexToUint8(str)];
		if (constants.sizes.addressEncoded === str.length)
			return ['address', new Address(str).bytes];

		throw Error(`invalid length of account id '${str.length}'`);
	},
	hash256: str => {
		if (2 * constants.sizes.hash256 === str.length)
			return utils.hexToUint8(str);

		throw Error(`invalid length of hash256 '${str.length}'`);
	},
	hash512: str => {
		if (2 * constants.sizes.hash512 === str.length)
			return utils.hexToUint8(str);

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
	 * Named parsers if it needs to be called directly.
	 */
	namedParserMap,
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
			const numericPageSize = utils.tryParseUint(args.pageSize);
			if (undefined === numericPageSize)
				throw errors.createInvalidArgumentError('pageSize is not a valid unsigned integer');

			parsedArgs.pageSize = getBoundedPageSize(numericPageSize, optionsPageSize);
		} else {
			parsedArgs.pageSize = optionsPageSize.default;
		}

		if (args.pageNumber) {
			const numericPageNumber = utils.tryParseUint(args.pageNumber);
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
		 * @returns {Function} An appropriate array handler.
		 */
		sendArray(id) {
			return array => {
				if (!Array.isArray(array))
					throw errors.createInternalError(`error retrieving data for id: '${id}'`);
				return { payload: array, type };
			};
		},

		/**
		 * Creates an object handler that either forwards an object corresponding to an identifier
		 * or throws a not found error if no such object exists.
		 * @param {object} id Object identifier.
		 * @returns {Function} An appropriate object handler.
		 */
		sendOne(id) {
			const resolveOne = object => {
				if (!object)
					throw errors.createResourceNotFoundError(id);
				return { payload: object, type };
			};

			return object => {
				if (Array.isArray(object)) {
					if (2 <= object.length)
						throw errors.createInternalError(`error retrieving data for id: '${id}' (length ${object.length})`);
					return resolveOne(object.length && object[0]);
				}
				return resolveOne(object);
			};
		},

		/**
		 * Creates a page handler that forwards a paginated result.
		 * @returns {Function} An appropriate object handler.
		 */
		sendPage() {
			return page => {
				if (!isPage(page))
					throw errors.createInternalError('error retrieving data');
				return { payload: page, type, structure: 'page' };
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

		server.get(routes.get, async (request, reply) => {
			const key = routeUtils.parseArgument(request.params, routeInfo.singular, parser);
			const result = await documentRetriever([key]);
			return reply.send(sender.sendOne(request.params[routeInfo.singular])(result));
		});

		server.post(routes.post, async (request, reply) => {
			const keys = routeUtils.parseArgumentAsArray(request.params, routeInfo.plural, parser);
			const result = await documentRetriever(keys);
			return reply.send(sender.sendArray(request.params[routeInfo.plural])(result));
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

		server.put(routeInfo.routeName, async (request, reply) => {
			const packetBuffer = createPacketFromBuffer(parser(request.params), routeInfo.packetType);
			await connections.lease().then(connection => connection.send(packetBuffer));
			reply.code(202).send({ message: `packet ${routeInfo.packetType} was pushed to the network via ${routeInfo.routeName}` });
		});
	},

	/**
	 * Returns function for processing merkle tree path requests.
	 * @param {module:db/CatapultDb} db Catapult database.
	 * @param {string} blockMetaCountField Field name for block meta count.
	 * @param {string} blockMetaTreeField Field name for block meta merkle tree.
	 * @returns {Function} Fastify-native async handler for merkle path requests.
	 */
	blockRouteMerkleProcessor: (db, blockMetaCountField, blockMetaTreeField) => async (request, reply) => {
		const height = routeUtils.parseArgument(request.params, 'height', 'uint64');
		const hash = routeUtils.parseArgument(request.params, 'hash', 'hash256');

		const result = await dbFacade.runHeightDependentOperation(
			db,
			height,
			() => db.blockWithMerkleTreeAtHeight(height, blockMetaTreeField)
		);

		if (!result.isRequestValid)
			throw errors.createResourceNotFoundError(height);

		const block = result.payload;
		const errorMessage = `hash '${request.params.hash}' not included in block height '${height}'`;
		if (!block.meta[blockMetaCountField])
			throw errors.createInvalidArgumentError(errorMessage);

		const merkleTree = {
			count: block.meta[blockMetaCountField],
			nodes: block.meta[blockMetaTreeField].map(merkleHash => merkleHash.buffer)
		};

		if (0 > indexOfLeafWithHash(hash, merkleTree))
			throw errors.createInvalidArgumentError(errorMessage);

		const merklePath = buildAuditPath(hash, merkleTree);
		return reply.send({ payload: { merklePath }, type: routeResultTypes.merkleProofInfo });
	}
};

export default routeUtils;
