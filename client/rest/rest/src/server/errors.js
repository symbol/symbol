const restifyErrors = require('restify-errors');

module.exports = {
	/**
	 * Converts an arbitrary error to a REST error.
	 * @param {Error} err The source error.
	 * @returns {Error} An appropriate REST error.
	 */
	toRestError: err => (err.statusCode
		? err
		: new restifyErrors.InternalError(err, err.message || 'unexpected error')),

	/**
	 * Creates a not found error.
	 * @param {object} id The id of the resource that couldn't be found.
	 * @returns {Error} An appropriate REST error.
	 */
	createNotFoundError: id => new restifyErrors.ResourceNotFoundError(`no resource exists with id '${id}'`),

	/**
	 * Creates an invalid argument error.
	 * @param {string} message The error message.
	 * @param {Error} err The optional invalid argument cause.
	 * @returns {Error} An appropriate REST error.
	 */
	createInvalidArgumentError: (message, err) => (err
		? new restifyErrors.InvalidArgumentError(err, message)
		: new restifyErrors.InvalidArgumentError(message)),

	/**
	 * Creates a service unavailable error.
	 * @param {string} message The error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createServiceUnavailableError: message => new restifyErrors.ServiceUnavailableError(message),

	/**
	 * Creates an internal error.
	 * @param {string} message The error message.
	 * @returns {Error} An appropriate REST error.
	 */
	createInternalError: message => new restifyErrors.InternalError(message)
};
