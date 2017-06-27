import restify from 'restify';

export default {
	toRestError: err => (err.statusCode
		? err
		: new restify.errors.InternalError(err, err.message || 'unexpected error')),

	createNotFoundError: id => new restify.errors.ResourceNotFoundError(`no resource exists with id '${id}'`),
	createInvalidArgumentError: message => new restify.errors.InvalidArgumentError(message),
	createServiceUnavailableError: message => new restify.errors.ServiceUnavailableError(message),
	createInternalError: message => new restify.errors.InternalError(message)
};
