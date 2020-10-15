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

const restifyErrors = require('restify-errors');

module.exports = {
	/**
	 * Converts an arbitrary error to a REST error.
	 * @param {Error} err Source error.
	 * @returns {Error} An appropriate REST error.
	 */
	toRestError: err => (err.statusCode
		? err
		: new restifyErrors.InternalError(err, err.message || 'unexpected error')),

	/**
	 * Creates a not found error.
	 * @param {object} id Id of the resource that couldn't be found.
	 * @returns {Error} An appropriate REST error.
	 */
	createNotFoundError: id => new restifyErrors.ResourceNotFoundError(`no resource exists with id '${id}'`),

	/**
	 * Creates an invalid argument error.
	 * @param {string} message Error message.
	 * @param {Error} err Optional invalid argument cause.
	 * @returns {Error} An appropriate REST error.
	 */
	createInvalidArgumentError: (message, err) => (err
		? new restifyErrors.InvalidArgumentError(err, message)
		: new restifyErrors.InvalidArgumentError(message)),

	/**
	 * Creates a service unavailable error.
	 * @param {string} message Error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createServiceUnavailableError: message => new restifyErrors.ServiceUnavailableError(message),

	/**
	 * Creates an internal error.
	 * @param {string} message Error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createInternalError: message => new restifyErrors.InternalError(message)
};
