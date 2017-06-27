import { expect } from 'chai';
import routeUtils from '../../src/routes/routeUtils';
import test from './utils/routeTestUtils';

const invalidObjectIdStrings = [
	'112233445566778899AABB', // too short
	'112233445566778899AABBCCDD', // too long
	'11223344556G778899AABBCC' // not hex (contains 'G')
];

describe('route utils', () => {
	describe('parse argument', () => {
		it('succeeds when parser does not error', () => {
			// Act:
			const result = routeUtils.parseArgument({ id: 7 }, 'id', id => id * 2);

			// Assert:
			expect(result).to.equal(14);
		});

		it('maps parser error to 409 error', () => {
			// Act:
			test.assert.invokerThrowsError(
				() => routeUtils.parseArgument({ id: 7 }, 'id', () => { throw Error('something bad happened'); }), {
					statusCode: 409,
					message: 'id has an invalid format: something bad happened'
				});
		});

		it('succeeds when object id parser does not error', () => {
			// Act:
			const result = routeUtils.parseArgument({ foo: '112233445566778899AABBCC' }, 'foo', 'objectId');

			// Assert:
			expect(result).to.equal('112233445566778899AABBCC');
		});

		it('maps object id parser error to 409 error', () => {
			// Act:
			for (const str of invalidObjectIdStrings) {
				test.assert.invokerThrowsError(
					() => routeUtils.parseArgument({ foo: str }, 'foo', 'objectId'), {
						statusCode: 409,
						message: 'foo has an invalid format: must be 12-byte hex string'
					});
			}
		});

		it('succeeds when uint parser does not error', () => {
			// Act:
			const result = routeUtils.parseArgument({ foo: '1234' }, 'foo', 'uint');

			// Assert:
			expect(result).to.equal(1234);
		});

		it('maps uint parser error to 409 error', () => {
			// Act:
			for (const str of ['-1', 'bar', '11223344556677889900']) {
				test.assert.invokerThrowsError(
					() => routeUtils.parseArgument({ foo: str }, 'foo', 'uint'), {
						statusCode: 409,
						message: 'foo has an invalid format: must be non-negative number'
					});
			}
		});
	});

	describe('parse argument array', () => {
		it('succeeds when parser does not error', () => {
			// Act:
			const result = routeUtils.parseArgumentAsArray({ ids: [5, 8, 13] }, 'ids', id => id * 2);

			// Assert:
			expect(result).to.deep.equal([10, 16, 26]);
		});

		it('non array passed to parser maps to 409 error', () => {
			// Act + Assert:
			test.assert.invokerThrowsError(
				() => routeUtils.parseArgumentAsArray({ ids: 1234 }, 'ids', id => id * 2), {
					statusCode: 409,
					message: 'ids has an invalid format: not an array'
				});
		});

		it('maps parser error to 409 error', () => {
			// Arrange:
			const throwIfEight = value => {
				if (8 === value)
					throw Error(`something bad happened, element ${value}`);

				return value + 1;
			};

			// Act + Assert:
			test.assert.invokerThrowsError(
				() => routeUtils.parseArgumentAsArray({ ids: [5, 8, 13] }, 'ids', throwIfEight), {
					statusCode: 409,
					message: 'element in array ids has an invalid format: something bad happened, element 8'
				});
		});
	});

	describe('parse paging arguments', () => {
		it('succeeds when no arguments are provided', () => {
			// Act:
			const options = routeUtils.parsePagingArguments({});

			// Assert:
			expect(options).to.deep.equal({ id: undefined, pageSize: 0 });
		});

		it('succeeds when valid id is provided', () => {
			// Act:
			const options = routeUtils.parsePagingArguments({ id: '112233445566778899AABBCC' });

			// Assert:
			expect(options).to.deep.equal({ id: '112233445566778899AABBCC', pageSize: 0 });
		});

		it('succeeds when valid page size is provided', () => {
			// Act:
			const options = routeUtils.parsePagingArguments({ pageSize: '12' });

			// Assert:
			expect(options).to.deep.equal({ id: undefined, pageSize: 12 });
		});

		it('succeeds when valid id and page size are provided', () => {
			// Act:
			const options = routeUtils.parsePagingArguments({ id: '112233445566778899AABBCC', pageSize: '12' });

			// Assert:
			expect(options).to.deep.equal({ id: '112233445566778899AABBCC', pageSize: 12 });
		});

		it('fails when invalid id is provided', () => {
			// Act:
			for (const str of invalidObjectIdStrings) {
				expect(() => routeUtils.parsePagingArguments({ id: str, pageSize: '12' }), `id ${str}`)
					.to.throw('id is not a valid object id');
			}
		});

		it('fails when invalid page size is provided', () => {
			// Act:
			expect(() => routeUtils.parsePagingArguments({ id: '112233445566778899AABBCC', pageSize: '1Y2' }))
				.to.throw('pageSize is not a valid unsigned integer');
		});
	});

	function sendTest(sender, assertResponse) {
		// Arrange: set up the route params
		const routeContext = { numNextCalls: 0 };
		const next = () => { ++routeContext.numNextCalls; };

		routeContext.responses = [];
		const res = { send: response => { routeContext.responses.push(response); } };

		// Act: send the entity
		sender(res, next);

		// Assert: exactly one response was sent
		expect(routeContext.numNextCalls).to.equal(1);
		expect(routeContext.responses.length).to.equal(1);
		assertResponse(routeContext.responses[0]);
	}

	describe('send entities', () => {
		function send(entity, type, assertResponse) {
			sendTest(
				(res, next) => routeUtils.sendEntities('dummyId', type, res, next)(entity),
				assertResponse);
		}

		it('forwards entities when defined', () => {
			// Act:
			send([{ alpha: 7 }], 'foo', response => {
				// Assert:
				expect(response).to.deep.equal({
					payload: [{ alpha: 7 }],
					type: 'foo'
				});
			});
		});

		it('sends error when entity is not an array', () => {
			// Act:
			send({ alpha: 7 }, 'foo', response => {
				// Assert:
				expect(response.body).to.deep.equal({ code: 'InternalError', message: 'error retrieving data for id: \'dummyId\'' });
			});
		});

		it('sends error when entity is undefined', () => {
			// Act:
			send(undefined, 'foo', response => {
				// Assert:
				expect(response.body).to.deep.equal({ code: 'InternalError', message: 'error retrieving data for id: \'dummyId\'' });
			});
		});
	});

	describe('send entity or not found', () => {
		function send(entity, id, type, assertResponse) {
			sendTest(
				(res, next) => routeUtils.sendEntityOrNotFound(id, type, res, next)(entity),
				assertResponse);
		}

		it('forwards entity when defined', () => {
			// Act:
			send({ alpha: 7 }, 'alpha-7', 'foo', response => {
				// Assert:
				expect(response).to.deep.equal({
					payload: { alpha: 7 },
					type: 'foo'
				});
			});
		});

		it('sends error when entity is undefined', () => {
			// Act:
			send(undefined, 'alpha-7', 'foo', response => {
				// Assert:
				expect(response.body).to.deep.equal({ code: 'ResourceNotFound', message: 'no resource exists with id \'alpha-7\'' });
			});
		});
	});
});
