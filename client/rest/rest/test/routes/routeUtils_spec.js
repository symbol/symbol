/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const { expect } = require('chai');
const catapult = require('catapult-sdk');
const routeUtils = require('../../src/routes/routeUtils');
const test = require('./utils/routeTestUtils');

const invalidObjectIdStrings = [
	'112233445566778899AABB', // too short
	'112233445566778899AABBCCDD', // too long
	'11223344556G778899AABBCC' // not hex (contains 'G')
];

describe('route utils', () => {
	describe('parse argument', () => {
		const addParserTests = traits => {
			it('succeeds when parser does not error', () => {
				traits.valid.forEach(valid => {
					// Act:
					const result = routeUtils.parseArgument({ foo: valid.id }, 'foo', traits.parser);

					// Assert:
					expect(result).to.deep.equal(valid.parsed);
				});
			});

			it('maps parser error to 409 error', () => {
				// Act:
				traits.invalid.forEach(invalid => {
					test.assert.invokerThrowsError(() => routeUtils.parseArgument({ foo: invalid.id }, 'foo', traits.parser), {
						statusCode: 409,
						message: 'foo has an invalid format'
					});
				});
			});
		};

		describe('custom', () => addParserTests({
			parser: x => {
				if (8 !== x)
					return x * 2;

				throw Error('something bad happened');
			},
			valid: [{ id: 7, parsed: 14 }],
			invalid: [{ id: 8, error: 'something bad happened' }]
		}));

		describe('objectId', () => addParserTests({
			parser: 'objectId',
			valid: [
				{ id: '112233445566778899AABBCC', parsed: '112233445566778899AABBCC' }
			],
			invalid: invalidObjectIdStrings.map(id => ({ id, error: 'must be 12-byte hex string' }))
		}));

		describe('uint', () => addParserTests({
			parser: 'uint',
			valid: [
				{ id: '1234', parsed: 1234 }
			],
			invalid: ['-1', 'bar', '11223344556677889900'].map(id => ({ id, error: 'must be non-negative number' }))
		}));

		const {
			addresses, publicKeys, hashes256, hashes512
		} = test.sets;

		describe('address', () => addParserTests({
			parser: 'address',
			valid: addresses.valid.map(id => ({ id, parsed: catapult.model.address.stringToAddress(id) })),
			invalid: [
				{ id: addresses.invalid, error: 'illegal base32 character 1' },
				{ id: '12345', error: 'invalid length of address \'5\'' }
			]
		}));

		describe('publicKey', () => addParserTests({
			parser: 'publicKey',
			valid: publicKeys.valid.map(id => ({ id, parsed: catapult.utils.convert.hexToUint8(id) })),
			invalid: [
				{ id: publicKeys.invalid, error: 'unrecognized hex char \'1G\'' },
				{ id: '12345', error: 'invalid length of publicKey \'5\'' }
			]
		}));

		describe('accountId', () => {
			describe('address', () => addParserTests({
				parser: 'accountId',
				valid: addresses.valid.map(id => ({ id, parsed: ['address', catapult.model.address.stringToAddress(id)] })),
				invalid: [
					{ id: addresses.invalid, error: 'illegal base32 character 1' }
				]
			}));

			describe('publicKey', () => addParserTests({
				parser: 'accountId',
				valid: publicKeys.valid.map(id => ({ id, parsed: ['publicKey', catapult.utils.convert.hexToUint8(id)] })),
				invalid: [
					{ id: publicKeys.invalid, error: 'unrecognized hex char \'1G\'' }
				]
			}));

			it('maps parser error to 409 error', () => {
				// Arrange
				const invalidAccountIds = [
					'012345678901234567890123456789012345678', // too short, 39 chars
					'01234567890123456789012345678901234567890123456789', // more than 40 but less than 64 chars
					'0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0' // more than 64 chars
				];

				// Act:
				invalidAccountIds.forEach(str => {
					test.assert.invokerThrowsError(() => routeUtils.parseArgument({ foo: str }, 'foo', 'accountId'), {
						statusCode: 409,
						message: 'foo has an invalid format'
					});
				});
			});
		});

		describe('hash256', () => addParserTests({
			parser: 'hash256',
			valid: hashes256.valid.map(hash => ({ id: hash, parsed: catapult.utils.convert.hexToUint8(hash) })),
			invalid: hashes256.invalid.map(hash => ({ id: hash, error: `invalid length of hash256 '${hash.length}` }))
		}));

		describe('hash512', () => addParserTests({
			parser: 'hash512',
			valid: hashes512.valid.map(hash => ({ id: hash, parsed: catapult.utils.convert.hexToUint8(hash) })),
			invalid: hashes512.invalid.map(hash => ({ id: hash, error: `invalid length of hash512 '${hash.length}` }))
		}));
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
			test.assert.invokerThrowsError(() => routeUtils.parseArgumentAsArray({ ids: 1234 }, 'ids', id => id * 2), {
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
			test.assert.invokerThrowsError(() => routeUtils.parseArgumentAsArray({ ids: [5, 8, 13] }, 'ids', throwIfEight), {
				statusCode: 409,
				message: 'element in array ids has an invalid format'
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
			invalidObjectIdStrings.forEach(str => {
				expect(() => routeUtils.parsePagingArguments({ id: str, pageSize: '12' }), `id ${str}`)
					.to.throw('id is not a valid object id');
			});
		});

		it('fails when invalid page size is provided', () => {
			// Act:
			expect(() => routeUtils.parsePagingArguments({ id: '112233445566778899AABBCC', pageSize: '1Y2' }))
				.to.throw('pageSize is not a valid unsigned integer');
		});
	});

	describe('generate valid page sizes', () => {
		it('can generate page sizes inclusive of min and max', () => {
			// Act:
			const pageSizes = routeUtils.generateValidPageSizes({ min: 30, max: 100, step: 10 });

			// Assert:
			expect(pageSizes).to.deep.equal([30, 40, 50, 60, 70, 80, 90, 100]);
		});

		it('can generate page sizes inclusive of min but not max', () => {
			// Act:
			const pageSizes = routeUtils.generateValidPageSizes({ min: 30, max: 99, step: 10 });

			// Assert:
			expect(pageSizes).to.deep.equal([30, 40, 50, 60, 70, 80, 90]);
		});

		it('can generate page sizes inclusive of max but not min', () => {
			// Act:
			const pageSizes = routeUtils.generateValidPageSizes({ min: 31, max: 100, step: 25 });

			// Assert:
			expect(pageSizes).to.deep.equal([50, 75, 100]);
		});

		it('can generate page sizes exclusive of min and max', () => {
			// Act:
			const pageSizes = routeUtils.generateValidPageSizes({ min: 10, max: 110, step: 25 });

			// Assert:
			expect(pageSizes).to.deep.equal([25, 50, 75, 100]);
		});

		it('can generate page sizes when only a single valid page size is configured', () => {
			// Act:
			const pageSizes = routeUtils.generateValidPageSizes({ min: 30, max: 45, step: 40 });

			// Assert:
			expect(pageSizes).to.deep.equal([40]);
		});

		it('cannot generate page sizes when there are no valid page sizes configured ', () => {
			// Arrange:
			const testCases = [
				{ config: { min: 30, max: 45, step: 25 }, desc: 'no step multiple is within range' },
				{ config: { min: 30, max: 45, step: 50 }, desc: 'step is greater than max' }
			];

			// Act:
			const errorMessage = 'page size configuration does not specify any valid page sizes';
			testCases.forEach(testCase => {
				expect(() => routeUtils.generateValidPageSizes(testCase.config), testCase.name).to.throw(errorMessage);
			});
		});
	});

	describe('sender', () => {
		const sendTest = (sender, assertResponse) => {
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
		};

		describe('send array', () => {
			const send = (object, id, type, assertResponse) => {
				sendTest((res, next) => routeUtils.createSender(type).sendArray(id, res, next)(object), assertResponse);
			};

			it('forwards array when defined', () => {
				// Act:
				send([{ alpha: 7 }], 'alpha-7', 'foo', response => {
					// Assert:
					expect(response).to.deep.equal({ payload: [{ alpha: 7 }], type: 'foo' });
				});
			});

			it('sends error when not array', () => {
				// Act:
				send({ alpha: 7 }, 'alpha-7', 'foo', response => {
					// Assert:
					expect(response.body).to.deep.equal({ code: 'Internal', message: 'error retrieving data for id: \'alpha-7\'' });
				});
			});

			it('sends error when array is undefined', () => {
				// Act:
				send(undefined, 'alpha-7', 'foo', response => {
					// Assert:
					expect(response.body).to.deep.equal({ code: 'Internal', message: 'error retrieving data for id: \'alpha-7\'' });
				});
			});
		});

		describe('send one', () => {
			const send = (object, id, type, assertResponse) => {
				sendTest((res, next) => routeUtils.createSender(type).sendOne(id, res, next)(object), assertResponse);
			};

			it('forwards object when defined', () => {
				// Act:
				send({ alpha: 7 }, 'alpha-7', 'foo', response => {
					// Assert:
					expect(response).to.deep.equal({ payload: { alpha: 7 }, type: 'foo' });
				});
			});

			it('sends error when object is undefined', () => {
				// Act:
				send(undefined, 'alpha-7', 'foo', response => {
					// Assert:
					expect(response.body).to.deep.equal({ code: 'ResourceNotFound', message: 'no resource exists with id \'alpha-7\'' });
				});
			});

			it('forwards object in single element array when defined', () => {
				// Act:
				send([{ alpha: 7 }], 'alpha-7', 'foo', response => {
					// Assert:
					expect(response).to.deep.equal({ payload: { alpha: 7 }, type: 'foo' });
				});
			});

			it('sends error when array is empty', () => {
				// Act:
				send([], 'alpha-7', 'foo', response => {
					// Assert:
					expect(response.body).to.deep.equal({ code: 'ResourceNotFound', message: 'no resource exists with id \'alpha-7\'' });
				});
			});

			it('sends error when array has multiple elements', () => {
				// Act:
				send([{ alpha: 7 }, { beta: 6 }], 'alpha-7', 'foo', response => {
					// Assert:
					expect(response.body).to.deep.equal({
						code: 'Internal',
						message: 'error retrieving data for id: \'alpha-7\' (length 2)'
					});
				});
			});
		});
	});

	describe('addGetPostDocumentRoutes', () => {
		const createRegistrar = postfixes => (server, db) => {
			const sender = routeUtils.createSender('bar');
			const routeInfo = {
				base: '/foo', singular: 'foo', plural: 'foos', postfixes
			};
			routeUtils.addGetPostDocumentRoutes(server, sender, routeInfo, keys => db.foos(keys), 'uint');
		};

		const errorMessage = 'has an invalid format';
		const basicRoutesDescriptor = {
			inputs: {
				valid: { object: { foo: '12345' }, parsed: [12345], printable: '12345' },
				validMultiple: { object: { foos: ['12345', '98765'] }, parsed: [12345, 98765] },
				invalid: { object: { foo: '12B45' }, error: `foo ${errorMessage}` },
				invalidMultiple: { object: { foos: ['12345', '12B45', '98765'] }, error: `element in array foos ${errorMessage}` }
			},
			dbApiName: 'foos',
			type: 'bar'
		};

		describe('default naming', () => {
			test.route.document.addGetPostDocumentRouteTests(
				createRegistrar(),
				Object.assign({ routes: { singular: '/foo/:foo', plural: '/foo' } }, basicRoutesDescriptor)
			);
		});

		describe('postfix naming', () => {
			test.route.document.addGetPostDocumentRouteTests(
				createRegistrar({ singular: 'token', plural: 'tokens' }),
				Object.assign({ routes: { singular: '/foo/:foo/token', plural: '/foo/tokens' } }, basicRoutesDescriptor)
			);
		});
	});

	describe('addPutPacketRoute', () => {
		const registrar = (server, db, services) => {
			const parseHexParam = (params, key) => routeUtils.parseArgument(params, key, catapult.utils.convert.hexToUint8);
			routeUtils.addPutPacketRoute(
				server,
				services.connections,
				{ routeName: '/foo/bar', packetType: 987 },
				params => Buffer.concat([parseHexParam(params, 'alpha'), parseHexParam(params, 'beta')])
			);
		};

		test.route.packet.addPutPacketRouteTests(registrar, {
			routeName: '/foo/bar',
			packetType: '987',
			inputs: {
				valid: {
					params: { alpha: '1234', beta: '9981AB' },
					parsed: Buffer.of(
						0x0D, 0x00, 0x00, 0x00, // size (header)
						0xDB, 0x03, 0x00, 0x00, // type (header)
						0x12, 0x34, 0x99, 0x81, 0xAB // payload (alpha, beta)
					)
				},
				invalid: {
					params: { alpha: '1234', beta: '9P81AB' },
					error: { key: 'beta' }
				}
			}
		});
	});
});
