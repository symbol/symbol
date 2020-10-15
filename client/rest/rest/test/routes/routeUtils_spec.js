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

const { test } = require('./utils/routeTestUtils');
const { convertToLong } = require('../../src/db/dbUtils');
const routeUtils = require('../../src/routes/routeUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { Binary, ObjectId } = MongoDb;
const { convert } = catapult.utils;

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

		describe('boolean', () => addParserTests({
			parser: 'boolean',
			valid: [{ id: 'true', parsed: true }, { id: 'false', parsed: false }],
			invalid: [{ id: 'abcd', error: 'must be boolean value \'true\' or \'false\'' }]
		}));

		describe('uint64', () => addParserTests({
			parser: 'uint64',
			valid: [
				{ id: '4468410971573743', parsed: [0x00ABCDEF, 0x000FDFFF] }
			],
			invalid: ['-43534534', '0DC67FBE1CAD29E'].map(id => ({ id }))
		}));

		describe('uint64hex', () => addParserTests({
			parser: 'uint64hex',
			valid: [
				{ id: '0DC67FBE1CAD29E3', parsed: [481110499, 231112638] }
			],
			invalid: ['0DC67FBE', '0DC67FBE1CAD29E3245', '0DC67FBE1CAD29ER'].map(id => ({ id }))
		}));
	});

	describe('parse argument array', () => {
		it('succeeds when parser does not error', () => {
			// Act:
			const result = routeUtils.parseArgumentAsArray({ ids: [5, 8, 13] }, 'ids', id => id * 2);

			// Assert:
			expect(result).to.deep.equal([10, 16, 26]);
		});

		it('non array passed to parser forcefully converts to array', () => {
			// Act:
			const result = routeUtils.parseArgumentAsArray({ ids: 1234 }, 'ids', id => id * 2);

			// Assert:
			expect(result).to.deep.equal([2468]);
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

	describe('parse pagination arguments', () => {
		const servicesConfigPageSize = {
			min: 10,
			max: 100,
			default: 20
		};

		const createObjectId = id => new ObjectId(`${'00'.repeat(12)}${id.toString(16)}`.slice(-24));

		it('succeeds when no arguments are provided', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments({}, servicesConfigPageSize, ['id']);

			// Assert:
			expect(options).to.deep.equal({
				pageSize: servicesConfigPageSize.default,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1
			});
		});

		it('succeeds when valid page size is provided', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments({ pageSize: '12' }, servicesConfigPageSize, { id: 'objectId' });

			// Assert:
			expect(options).to.deep.equal({
				pageSize: 12,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1
			});
		});

		it('binds page size to max page size', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments(
				{ pageSize: (servicesConfigPageSize.max + 1).toString() },
				servicesConfigPageSize,
				{ id: 'objectId' }
			);

			// Assert:
			expect(options).to.deep.equal({
				pageSize: servicesConfigPageSize.max,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1
			});
		});

		it('binds page size to min page size', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments(
				{ pageSize: (servicesConfigPageSize.min - 1).toString() },
				servicesConfigPageSize,
				{ id: 'objectId' }
			);

			// Assert:
			expect(options).to.deep.equal({
				pageSize: servicesConfigPageSize.min,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1
			});
		});

		it('succeeds when valid page number is provided', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments({ pageNumber: '5' }, servicesConfigPageSize, { id: 'objectId' });

			// Assert:
			expect(options).to.deep.equal({
				pageSize: servicesConfigPageSize.default,
				pageNumber: 5,
				sortField: 'id',
				sortDirection: 1
			});
		});

		it('defaults page number to 1 when 0 is provided', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments({ pageNumber: '0' }, servicesConfigPageSize, { id: 'objectId' });

			// Assert:
			expect(options).to.deep.equal({
				pageSize: servicesConfigPageSize.default,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1
			});
		});

		it('succeeds when valid sort field is provided', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments(
				{ orderBy: 'hash' },
				servicesConfigPageSize,
				{ id: 'objectId', hash: 'hash256' }
			);

			// Assert:
			expect(options).to.deep.equal({
				pageSize: servicesConfigPageSize.default,
				pageNumber: 1,
				sortField: 'hash',
				sortDirection: 1
			});
		});

		it('uses `id` sort field as default', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments({}, servicesConfigPageSize, ['address', 'key']);

			// Assert:
			expect(options).to.deep.equal({
				pageSize: servicesConfigPageSize.default,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1
			});
		});

		describe('succeeds when valid sort direction is provided', () => {
			const runSortDirectionTest = (argName, order, expectedValue) => {
				it(`sort argument: ${argName}`, () => {
					// Act:
					const options = routeUtils.parsePaginationArguments({ order }, servicesConfigPageSize, { id: 'objectId' });

					// Assert:
					expect(options).to.deep.equal({
						pageSize: servicesConfigPageSize.default,
						pageNumber: 1,
						sortField: 'id',
						sortDirection: expectedValue
					});
				});
			};

			runSortDirectionTest('asc', 'asc', 1);
			runSortDirectionTest('desc', 'desc', -1);
			runSortDirectionTest('other', 'abcd', 1);
		});

		it('succeeds when valid offset is provided', () => {
			// Arrange
			const offset = createObjectId(123).toString();

			// Act:
			const options = routeUtils.parsePaginationArguments({ offset }, servicesConfigPageSize, { id: 'objectId' });

			// Assert:
			expect(options).to.deep.equal({
				pageSize: servicesConfigPageSize.default,
				pageNumber: 1,
				sortField: 'id',
				sortDirection: 1,
				offset,
				offsetType: 'objectId'
			});
		});

		it('succeeds when valid page size and page number are provided', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments(
				{ pageSize: '12', pageNumber: '5' },
				servicesConfigPageSize,
				{ id: 'objectId' }
			);

			// Assert:
			expect(options).to.deep.equal({
				pageSize: 12,
				pageNumber: 5,
				sortField: 'id',
				sortDirection: 1
			});
		});

		it('succeeds when valid page size, page number, sort field and sort direction are provided', () => {
			// Act:
			const options = routeUtils.parsePaginationArguments({
				pageSize: '12',
				pageNumber: '5',
				orderBy: 'signerPublicKey',
				order: 'desc'
			}, servicesConfigPageSize, { signerPublicKey: 'publicKey' });

			// Assert:
			expect(options).to.deep.equal({
				pageSize: 12,
				pageNumber: 5,
				sortField: 'signerPublicKey',
				sortDirection: -1
			});
		});

		it('succeeds when valid page size, page number, sort field, sort direction and offset are provided', () => {
			// Arrange
			const offset = test.sets.publicKeys.valid[0];

			// Act:
			const options = routeUtils.parsePaginationArguments({
				pageSize: '12',
				pageNumber: '5',
				orderBy: 'signerPublicKey',
				order: 'desc',
				offset
			}, servicesConfigPageSize, { signerPublicKey: 'publicKey' });

			// Assert:
			expect(options).to.deep.equal({
				pageSize: 12,
				pageNumber: 5,
				sortField: 'signerPublicKey',
				sortDirection: -1,
				offset: convert.hexToUint8(offset),
				offsetType: 'publicKey'
			});
		});

		it('fails when invalid page size is provided', () => {
			// Act:
			expect(() => routeUtils.parsePaginationArguments({ pageSize: '1Y2' }, servicesConfigPageSize, { id: 'objectId' }))
				.to.throw('pageSize is not a valid unsigned integer');
		});

		it('fails when invalid page number is provided', () => {
			// Act:
			expect(() => routeUtils.parsePaginationArguments({ pageNumber: '12aa' }, servicesConfigPageSize, { id: 'objectId' }))
				.to.throw('pageNumber is not a valid unsigned integer');
		});

		it('fails when invalid sort field is provided', () => {
			// Act:
			expect(() => routeUtils.parsePaginationArguments(
				{ orderBy: 'hash' },
				servicesConfigPageSize,
				{ id: 'objectId', address: 'address' }
			)).to.throw('sorting by hash is not allowed');
		});

		it('fails when invalid offset is provided', () => {
			// Act:
			expect(() => routeUtils.parsePaginationArguments({ offset: '1Y2' }, servicesConfigPageSize, { id: 'objectId' }))
				.to.throw('offset has an invalid format');
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

		describe('send page', () => {
			const send = (object, type, assertResponse) => {
				sendTest((res, next) => routeUtils.createSender(type).sendPage(res, next)(object), assertResponse);
			};

			it('forwards valid page object', () => {
				// Act:
				const page = {
					data: [{}],
					pagination: {
						pageNumber: 1,
						pageSize: 10
					}
				};

				send(page, 'foo', response => {
					// Assert:
					expect(response).to.deep.equal({ payload: page, type: 'foo', structure: 'page' });
				});
			});

			it('forwards valid empty page object', () => {
				// Act:
				const page = {
					data: [],
					pagination: {
						pageNumber: 1,
						pageSize: 10
					}
				};

				send(page, 'foo', response => {
					// Assert:
					expect(response).to.deep.equal({ payload: page, type: 'foo', structure: 'page' });
				});
			});

			it('sends error when object is not a page', () => {
				// Act:
				send({ alpha: 7 }, 'foo', response => {
					// Assert:
					expect(response.body).to.deep.equal({ code: 'Internal', message: 'error retrieving data' });
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

	describe('blockRouteMerkleProcessor', () => {
		// Arrange:
		const highestHeight = 50;

		const sendFake = sinon.fake();
		const nextFake = sinon.fake();

		const formatHashAsBinary = hash => test.factory.createBinary(Buffer.from(convert.hexToUint8(hash), 'hex'));
		const formatBinaryAsHash = binary => convert.uint8ToHex(binary.buffer);
		const merkleTree = [
			formatHashAsBinary('9922093F19F7160BDCBCA8AA48499DA8DF532D4102745670B85AA4BDF63B8D59'),
			formatHashAsBinary('E8FCFD95CA220D442BE748F5494001A682DC8015A152EBC433222136E99A96B8'),
			formatHashAsBinary('C1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876'),
			formatHashAsBinary('F8E8FCDAD1B94D2C76D769B113FF5CAC5D5170772F2D80E466EB04FCA23D6887'),
			formatHashAsBinary('2D3418274BBC250616223C162534B460216AED82C4FA9A87B53083B7BA7A9391'),
			formatHashAsBinary('AEAF30ED55BBE4805C53E5232D88242F0CF719F99A8E6D339BCBF5D5DE85E1FB'),
			formatHashAsBinary('AFE6C917BABA60ADC1512040CC35033B563DAFD1718FA486AB1F3D9B84866B27')
		];
		const blockMetaCountField = 'blockMetaCountField';
		const blockMetaTreeField = 'blockMetaTreeField';

		const blockInfoMockData = { meta: {} };
		blockInfoMockData.meta[blockMetaCountField] = 4;
		blockInfoMockData.meta[blockMetaTreeField] = merkleTree;

		const db = {
			chainStatisticCurrent: () => Promise.resolve({ height: convertToLong(highestHeight) }),
			blockWithMerkleTreeAtHeight: () => Promise.resolve(blockInfoMockData)
		};

		const processorFunction = routeUtils.blockRouteMerkleProcessor(
			db,
			blockMetaCountField,
			blockMetaTreeField
		);

		beforeEach(() => {
			sendFake.resetHistory();
			nextFake.resetHistory();
		});

		it('returns a merkle path for valid height and hash', () => {
			// Arrange:
			const req = { params: { height: highestHeight.toString(), hash: formatBinaryAsHash(merkleTree[2]) } };

			// Act:
			return processorFunction(req, { send: sendFake }, nextFake).then(() => {
				// Assert:
				expect(sendFake.calledOnceWith(sinon.match({
					payload: {
						merklePath: [
							{ position: catapult.crypto.merkle.NodePosition.right, hash: merkleTree[3].buffer },
							{ position: catapult.crypto.merkle.NodePosition.left, hash: merkleTree[4].buffer }
						]
					},
					type: 'merkleProofInfo'
				}))).to.equal(true);
				expect(nextFake.calledOnce).to.equal(true);
			});
		});

		it('throws error if height has an invalid format', () => {
			// Arrange:
			const req = { params: { height: 'abc', hash: formatBinaryAsHash(merkleTree[2]) } };

			// Act + Assert:
			expect(processorFunction.bind(processorFunction, req, { send: sendFake }, nextFake))
				.to.throw('height has an invalid format');
		});

		it('throws error if hash has an invalid format', () => {
			// Arrange:
			const req = { params: { height: highestHeight.toString(), hash: 'AFE6C917' } };

			// Act + Assert:
			expect(processorFunction.bind(processorFunction, req, { send: sendFake }, nextFake))
				.to.throw('hash has an invalid format');
		});

		it('returns resource not found error if there is no block at this height', () => {
			// Arrange:
			const queriedHeight = highestHeight + 10;
			const queriedHash = formatBinaryAsHash(merkleTree[2]);
			const req = { params: { height: queriedHeight.toString(), hash: queriedHash } };

			// Act:
			return processorFunction(req, { send: sendFake }, nextFake).then(() => {
				// Assert:
				expect(sendFake.firstCall.args[0].body).to.deep.equal({
					code: 'ResourceNotFound',
					message: `no resource exists with id '${queriedHeight}'`
				});
				expect(nextFake.calledOnce).to.equal(true);
			});
		});

		it('returns invalid argument if hash is not included in block at this height', () => {
			// Arrange:
			const req = { params: { height: highestHeight.toString(), hash: formatBinaryAsHash(merkleTree[2]) } };
			blockInfoMockData.meta[blockMetaCountField] = 0;

			// Act:
			return processorFunction(req, { send: sendFake }, nextFake).then(() => {
				// Assert:
				expect(sendFake.firstCall.args[0].body).to.deep.equal({
					code: 'InvalidArgument',
					message: `hash '${req.params.hash}' not included in block height '${highestHeight}'`
				});
				expect(nextFake.calledOnce).to.equal(true);
				// restore data for following tests
				blockInfoMockData.meta[blockMetaCountField] = 4;
			});
		});

		it('returns invalid argument if hash is not found in merkle tree', () => {
			// Arrange:
			const req = {
				params: {
					height: highestHeight.toString(),
					hash: 'AAAAA62C63CAB4197C87B36605AAAAAF4FEAE575D81A7F728F4E3704613AAAAA'
				}
			};

			// Act:
			return processorFunction(req, { send: sendFake }, nextFake).then(() => {
				// Assert:
				expect(sendFake.firstCall.args[0].body).to.deep.equal({
					code: 'InvalidArgument',
					message: `hash '${req.params.hash}' not included in block height '${highestHeight}'`
				});
				expect(nextFake.calledOnce).to.equal(true);
			});
		});
	});

	describe('addressToPublicKey', () => {
		const { addresses, publicKeys } = test.sets;
		const accountAddress = catapult.model.address.stringToAddress(addresses.valid[0]);
		const accountPublicKey = convert.hexToUint8(publicKeys.valid[0]);

		it('return correct public key from account address ', () => {
			// Arrange:
			const dbAddressToPublicKeyFake = sinon.fake.resolves({
				_id: undefined,
				account: { publicKey: new Binary(Buffer.from(accountPublicKey)) }
			});
			const db = { addressToPublicKey: dbAddressToPublicKeyFake };
			// Act:
			return routeUtils.addressToPublicKey(db, accountAddress).then(result => {
				// Assert:
				expect(dbAddressToPublicKeyFake.calledOnceWith(accountAddress)).to.equal(true);
				expect(result.equals(accountPublicKey)).to.be.equal(true);
			});
		});

		it('rejects with error when account id is not found', () => {
			// Arrange:
			const dbAddressToPublicKeyFake = sinon.fake.resolves(undefined);
			const db = { addressToPublicKey: dbAddressToPublicKeyFake };
			// Act:
			return routeUtils.addressToPublicKey(db, accountAddress)
				// Assert:
				.then(() => expect.fail())
				.catch(err => {
					expect(err.toString()).to.include('account not found');
				});
		});
	});
});
