import { expect } from 'chai';
import restify from 'restify';
import errors from '../../src/server/errors';

describe('errors', () => {
	describe('toRestError', () => {
		it('can map basic error without message', () => {
			// Act:
			const err = errors.toRestError(new Error());

			// Assert:
			expect(err.statusCode).to.equal(500);
			expect(err.body).to.deep.equal({ code: 'InternalError', message: 'unexpected error' });
		});

		it('can map basic error with message', () => {
			// Act:
			const err = errors.toRestError(new Error('badness'));

			// Assert:
			expect(err.statusCode).to.equal(500);
			expect(err.body).to.deep.equal({ code: 'InternalError', message: 'badness' });
		});

		it('can map rest error', () => {
			// Act:
			const err = new restify.errors.NotFoundError('not found');

			// Assert:
			expect(err.statusCode).to.equal(404);
			expect(err.body).to.deep.equal({ code: 'NotFoundError', message: 'not found' });
		});
	});

	describe('create', () => {
		it('can create not found error', () => {
			// Act:
			const err = errors.createNotFoundError('foo');

			// Assert:
			expect(err.statusCode).to.equal(404);
			expect(err.body).to.deep.equal({ code: 'ResourceNotFound', message: 'no resource exists with id \'foo\'' });
		});

		it('can create invalid argument error', () => {
			// Act:
			const err = errors.createInvalidArgumentError('badness');

			// Assert:
			expect(err.statusCode).to.equal(409);
			expect(err.body).to.deep.equal({ code: 'InvalidArgument', message: 'badness' });
		});

		it('can create service unavailable error', () => {
			// Act:
			const err = errors.createServiceUnavailableError('badness');

			// Assert:
			expect(err.statusCode).to.equal(503);
			expect(err.body).to.deep.equal({ code: 'ServiceUnavailableError', message: 'badness' });
		});

		it('can create internal error', () => {
			// Act:
			const err = errors.createInternalError('badness');

			// Assert:
			expect(err.statusCode).to.equal(500);
			expect(err.body).to.deep.equal({ code: 'InternalError', message: 'badness' });
		});
	});
});
