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

const errors = require('../../src/server/errors');
const { expect } = require('chai');
const restifyErrors = require('restify-errors');

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
			const err = errors.toRestError(new Error('badness'));

			// Assert:
			expect(err.statusCode).to.equal(500);
			expect(err.body).to.deep.equal({ code: 'Internal', message: 'badness' });
		});

		it('can map rest error', () => {
			// Act:
			const err = new restifyErrors.NotFoundError('not found');

			// Assert:
			expect(err.statusCode).to.equal(404);
			expect(err.body).to.deep.equal({ code: 'NotFound', message: 'not found' });
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
			expect(err.jse_cause).to.equal(undefined);
		});

		it('can create invalid argument error with cause', () => {
			// Act:
			const err = errors.createInvalidArgumentError('badness', new Error('foo'));

			// Assert:
			expect(err.statusCode).to.equal(409);
			expect(err.body).to.deep.equal({ code: 'InvalidArgument', message: 'badness' });
			expect(err.jse_cause).to.not.equal(undefined);
			expect(err.jse_cause.message).to.equal('foo');
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
	});
});
