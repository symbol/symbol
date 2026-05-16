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

import errors from '../../src/server/errors.js';
import { expect } from 'chai';

describe('errors', () => {
	describe('toRestError', () => {
		it('can map basic error without message', () => {
			// Act:
			const err = errors.toRestError(new Error());

			// Assert:
			expect(err.statusCode).to.equal(500);
			expect(err.body).to.deep.equal({ code: 'Internal', message: 'unexpected error' });
		});

		it('can map basic error with message', () => {
			// Act:
			const originalError = new Error('badness');
			const err = errors.toRestError(originalError);

			// Assert:
			expect(err.statusCode).to.equal(500);
			expect(err.body).to.deep.equal({ code: 'Internal', message: 'badness' });
			expect(err.cause).to.deep.equal(originalError);
		});

		it('returns original error when already a rest error', () => {
			// Arrange:
			const originalError = {
				statusCode: 400,
				body: { code: 'user', message: 'test error' }
			};

			// Act:
			const err = errors.toRestError(originalError);

			// Assert:
			expect(err).to.deep.equal(originalError);
			expect(err.cause).to.equal(undefined);
		});
	});

	describe('create', () => {
		const assertTestNotFoundError = expectedMessage => {
			// Act:
			const err = expectedMessage ? errors.createNotFoundError(expectedMessage) : errors.createNotFoundError();

			// Assert:
			expect(err.statusCode).to.equal(404);
			expect(err.body).to.deep.equal({ code: 'NotFound', message: expectedMessage || '' });
		};

		it('can create not found error with default message', () => assertTestNotFoundError());

		it('can create not found error', () => assertTestNotFoundError('Not Found'));

		it('can create resource not found error', () => {
			// Act:
			const err = errors.createResourceNotFoundError('foo');

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
			expect(err.cause).to.equal(undefined);
		});

		it('can create invalid argument error with cause', () => {
			// Act:
			const err = errors.createInvalidArgumentError('badness', new Error('foo'));

			// Assert:
			expect(err.statusCode).to.equal(409);
			expect(err.body).to.deep.equal({ code: 'InvalidArgument', message: 'badness' });
			expect(err.cause).to.not.equal(undefined);
			expect(err.cause.message).to.equal('foo');
		});

		it('can create service unavailable error', () => {
			// Act:
			const err = errors.createServiceUnavailableError('badness');

			// Assert:
			expect(err.statusCode).to.equal(503);
			expect(err.body).to.deep.equal({ code: 'ServiceUnavailable', message: 'badness' });
		});

		it('can create internal error', () => {
			// Act:
			const err = errors.createInternalError('badness');

			// Assert:
			expect(err.statusCode).to.equal(500);
			expect(err.body).to.deep.equal({ code: 'Internal', message: 'badness' });
		});

		it('can create unsupported media type error', () => {
			// Act:
			const err = errors.createUnsupportedMediaTypeError('badness');

			// Assert:
			expect(err.statusCode).to.equal(415);
			expect(err.body).to.deep.equal({ code: 'UnsupportedMediaType', message: 'badness' });
		});

		it('can create not acceptable error', () => {
			// Act:
			const err = errors.createNotAcceptableError('badness');

			// Assert:
			expect(err.statusCode).to.equal(406);
			expect(err.body).to.deep.equal({ code: 'NotAcceptable', message: 'badness' });
		});
	});
});
