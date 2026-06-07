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

/**
 * Lightweight HTTP error with a `statusCode` and a `body` shaped as `{ code, message }`.
 * @param {number} statusCode HTTP status code.
 * @param {string} code Error code.
 * @param {string} message Error message.
 * @param {Error} cause Optional cause.
 */
class HttpError extends Error {
	constructor(statusCode, code, message, cause) {
		super(message);
		this.statusCode = statusCode;
		this.body = { code, message };
		if (cause)
			this.cause = cause;
	}
}

export default {
	/**
	 * Converts an arbitrary error to a REST error.
	 * @param {Error} err Source error.
	 * @returns {Error} An appropriate REST error.
	 */
	toRestError: err => (err.statusCode
		? err
		: new HttpError(500, 'Internal', err.message || 'unexpected error', err)),

	/**
	 * Creates a resource not found error.
	 * @param {object} id Id of the resource that couldn't be found.
	 * @returns {Error} An appropriate REST error.
	 */
	createResourceNotFoundError: id => new HttpError(404, 'ResourceNotFound', `no resource exists with id '${id}'`),

	/**
	 * Creates a not found error.
	 * @param {string} message Error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createNotFoundError: (message = '') => new HttpError(404, 'NotFound', message),

	/**
	 * Creates an invalid argument error.
	 * @param {string} message Error message.
	 * @param {Error} err Optional invalid argument cause.
	 * @returns {Error} An appropriate REST error.
	 */
	createInvalidArgumentError: (message, err) => new HttpError(409, 'InvalidArgument', message, err),

	/**
	 * Creates a service-unavailable error.
	 * @param {string} message Error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createServiceUnavailableError: message => new HttpError(503, 'ServiceUnavailable', message),

	/**
	 * Creates an internal error.
	 * @param {string} message Error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createInternalError: message => new HttpError(500, 'Internal', message),

	/**
	 * Creates an unsupported media type error.
	 * @param {string} message Error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createUnsupportedMediaTypeError: message => new HttpError(415, 'UnsupportedMediaType', message),

	/**
	 * Creates an unacceptable error.
	 * @param {string} message Error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createNotAcceptableError: message => new HttpError(406, 'NotAcceptable', message)
};
