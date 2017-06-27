import catapult from 'catapult-sdk';
import errors from '../server/errors';

const convert = catapult.utils.convert;

function isObjectId(str) {
	return 24 === str.length && convert.isHexString(str);
}

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
	}
};

export default {
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
			throw errors.createInvalidArgumentError(`${key} has an invalid format: ${err.message}`);
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
			throw errors.createInvalidArgumentError(`element in array ${key} has an invalid format: ${err.message}`);
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

		for (const key of Object.keys(parsedOptions)) {
			if (args[key]) {
				const parser = parsers[key];
				parsedOptions[key] = parser.tryParse(args[key]);
				if (!parsedOptions[key])
					throw errors.createInvalidArgumentError(`${key} is not a valid ${parser.type}`);
			}
		}

		return parsedOptions;
	},

	/**
	 * Creates an entity handler that forwards an entity.
	 * @param {object} id The entity identifier.
	 * @param {module:routes/routeResultTypes} type The entity type.
	 * @param {object} res The restify response object.
	 * @param {Function} next The restify next callback handler.
	 * @returns {Function} Entity handler.
	 */
	sendEntities(id, type, res, next) {
		return entity => {
			if (!Array.isArray(entity))
				res.send(errors.createInternalError(`error retrieving data for id: '${id}'`));
			else
				res.send({ payload: entity, type });

			next();
		};
	},

	/**
	 * Creates an entity handler that either forwards an entity corresponding to an identifier
	 * or sends a not found error if no such entity exists.
	 * @param {object} id The entity identifier.
	 * @param {module:routes/routeResultTypes} type The entity type.
	 * @param {object} res The restify response object.
	 * @param {Function} next The restify next callback handler.
	 * @returns {Function} An appropriate entity handler.
	 **/
	sendEntityOrNotFound(id, type, res, next) {
		return entity => {
			if (!entity)
				res.send(errors.createNotFoundError(id));
			else
				res.send({ payload: entity, type });

			next();
		};
	}
};
