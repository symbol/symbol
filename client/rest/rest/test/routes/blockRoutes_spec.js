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

const { MockServer, test } = require('./utils/routeTestUtils');
const blockRoutes = require('../../src/routes/blockRoutes');
const routeUtils = require('../../src/routes/routeUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { PacketType, StatePathPacketTypes } = catapult.packet;
const { convert } = catapult.utils;

describe('block routes', () => {
	const addChainStatisticToDb = db => { db.chainStatisticCurrent = () => Promise.resolve({ height: 10 }); };
	const routeConfig = { pageSize: { min: 30, max: 80 } };

	const serviceCreator = packet => ({
		connections: {
			singleUse: () => new Promise(resolve => {
				resolve({
					pushPull: () => new Promise(innerResolve => innerResolve(packet))
				});
			})
		},
		config: {
			apiNode: { timeout: 1000 }
		}
	});

	describe('block', () => {
		const builder = test.route.document.prepareGetDocumentRouteTests(blockRoutes.register, {
			route: '/block/:height',
			dbApiName: 'blockAtHeight',
			type: 'blockHeaderWithMetadata',
			extendDb: addChainStatisticToDb,
			config: routeConfig
		});
		builder.addDefault({
			valid: { object: { height: '3' }, parsed: [3], printable: '3' },
			invalid: { object: { height: '10A' }, error: 'height has an invalid format' }
		});
		builder.addNotFoundInputTest({ object: { height: '11' }, parsed: [11], printable: '11' }, 'chain height is too small');
	});

	describe('blocks from height', () => {
		const builder = test.route.document.prepareGetDocumentsRouteTests(blockRoutes.register, {
			route: '/blocks/:height/limit/:limit',
			dbApiName: 'blocksFrom',
			type: 'blockHeaderWithMetadata',
			config: routeConfig
		});

		builder.addValidInputTest({ object: { height: '1', limit: '80' }, parsed: [1, 80] }, '(default limit)');
		builder.addValidInputTest({ object: { height: '3601', limit: '72' }, parsed: [3601, 72] }, '(custom valid limit)');
		builder.addEmptyArrayTest({ object: { height: '1', limit: '40' }, parsed: [1, 40] });

		// notice that this expands to four tests { 'height', 'limit'} x { '10A', '-4321' }
		['height', 'limit'].forEach(property => ['10A', '-4321'].forEach(value => {
			const object = Object.assign({ height: '1234', limit: '4321' }, { [property]: value });
			const errorMessage = `${property} has an invalid format`;
			builder.addInvalidKeyTest({ object, error: errorMessage }, `(${property} with value ${value})`);
		}));

		builder.addRedirectTest({ object: { height: '0', limit: '0' }, redirectUri: '/blocks/1/limit/30' }, '{ height: 0, limit: 0 }');

		// limit is sanitized correctly
		builder.addRedirectTest(
			{ object: { height: '3601', limit: '0' }, redirectUri: '/blocks/3601/limit/30' },
			'{ height: 3601, limit: 0 }'
		);

		builder.addRedirectTest(
			{ object: { height: '3601', limit: '29' }, redirectUri: '/blocks/3601/limit/30' },
			'{ height: 3601, limit: 29 }'
		);

		builder.addRedirectTest(
			{ object: { height: '3601', limit: '81' }, redirectUri: '/blocks/3601/limit/80' },
			'{ height: 3601, limit: 81 }'
		);

		builder.addRedirectTest(
			{ object: { height: '3601', limit: '100' }, redirectUri: '/blocks/3601/limit/80' },
			'{ height: 3601, limit: 100 }'
		);
	});

	describe('block transactions', () => {
		// Arrange:
		const highestHeight = 100;
		const fakeTransactions = [
			{ meta: { addresses: [] }, transaction: { type: 12345 } }
		];
		const dbTransactionsAtHeightFake = sinon.fake.resolves(fakeTransactions);
		const db = {
			transactionsAtHeight: dbTransactionsAtHeightFake,
			chainStatisticCurrent: () => Promise.resolve({ height: highestHeight })
		};
		const mockServer = new MockServer();
		blockRoutes.register(mockServer.server, db, { config: routeConfig });

		beforeEach(() => {
			mockServer.resetStats();
			dbTransactionsAtHeightFake.resetHistory();
		});
		const route = mockServer.getRoute('/block/:height/transactions').get();

		it('basic query', () => {
			// Arrange:
			const req = { params: { height: '10' } };

			// Act:
			return mockServer.callRoute(route, req).then(() => {
				// Assert:
				expect(dbTransactionsAtHeightFake.calledOnce).to.equal(true);
				expect(dbTransactionsAtHeightFake.firstCall.args[0]).to.deep.equal(10);

				expect(mockServer.send.firstCall.args[0]).to.deep.equal({
					payload: fakeTransactions,
					type: 'transactionWithMetadata'
				});
				expect(mockServer.next.calledOnce).to.equal(true);
			});
		});

		it('parses and fordwards params correctly', () => {
			// Arrange:
			const req = {
				params: {
					height: '10',
					type: ['16724', '16717', '16973'],
					id: '00123456789AABBBCCDDEEFF',
					pageSize: '25'
				}
			};

			// Act:
			return mockServer.callRoute(route, req).then(() => {
				// Assert:
				expect(dbTransactionsAtHeightFake.calledOnce).to.equal(true);
				expect(dbTransactionsAtHeightFake.firstCall.args[1]).to.deep.equal([16724, 16717, 16973]);
				expect(dbTransactionsAtHeightFake.firstCall.args[2]).to.equal('00123456789AABBBCCDDEEFF');
				expect(dbTransactionsAtHeightFake.firstCall.args[3]).to.equal(25);
			});
		});

		it('returns 409 if invalid height', () => {
			// Arrange:
			const req = { params: { height: 'abc' } };

			// Act + Assert:
			expect(() => mockServer.callRoute(route, req)).to.throw('height has an invalid format');
		});

		it('returns 409 if invalid pageId', () => {
			// Arrange:
			const req = { params: { height: 10, id: '12345' } };

			// Act + Assert:
			expect(() => mockServer.callRoute(route, req)).to.throw('id is not a valid object id');
		});

		it('returns 409 if invalid pageSize', () => {
			// Arrange:
			const req = { params: { height: '10', id: '00123456789AABBBCCDDEEFF', pageSize: '-1' } };

			// Act + Assert:
			expect(() => mockServer.callRoute(route, req)).to.throw('pageSize is not a valid unsigned integer');
		});
	});

	describe('block with merkle tree', () => {
		it('calls blockRouteMerkleProcessor with correct params', () => {
			// Arrange:
			const mockServer = new MockServer();
			const blockRouteMerkleProcessorSpy = sinon.spy(routeUtils, 'blockRouteMerkleProcessor');

			// Act:
			blockRoutes.register(mockServer.server, {}, { config: routeConfig });

			// Assert:
			expect(blockRouteMerkleProcessorSpy.calledOnce).to.equal(true);
			expect(blockRouteMerkleProcessorSpy.firstCall.args[1]).to.equal('numTransactions');
			expect(blockRouteMerkleProcessorSpy.firstCall.args[2]).to.equal('transactionMerkleTree');
			blockRouteMerkleProcessorSpy.restore();
		});
	});

	describe('block state tree', () => {
		describe('returns the requested tree with valid params', () => {
			// Arrange:
			const stateTree = [
				'9922093F19F7160BDCBCA8AA48499DA8DF532D4102745670B85AA4BDF63B8D59',
				'E8FCFD95CA220D442BE748F5494001A682DC8015A152EBC433222136E99A96B8',
				'C1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876',
				'F8E8FCDAD1B94D2C76D769B113FF5CAC5D5170772F2D80E466EB04FCA23D6887',
				'2D3418274BBC250616223C162534B460216AED82C4FA9A87B53083B7BA7A9391',
				'AEAF30ED55BBE4805C53E5232D88242F0CF719F99A8E6D339BCBF5D5DE85E1FB',
				'AFE6C917BABA60ADC1512040CC35033B563DAFD1718FA486AB1F3D9B84866B27'
			].map((treeNode => Buffer.from(convert.hexToUint8(treeNode))));

			const packet = {
				type: PacketType.accountStatePath,
				size: 168,
				payload: Buffer.concat(stateTree)
			};
			const services = serviceCreator(packet);

			StatePathPacketTypes.forEach(packetType => {
				// Act:
				it(`for ${packetType} state`, () =>
					test.route.prepareExecuteRoute(
						blockRoutes.register,
						'/state/:state/hash/:hash/merkle',
						'get',
						{ state: packetType, hash: '2D3418274BBC250616223C162534B460216AED82C4FA9A87B53083B7BA7A9391' },
						{}, services, routeContext => routeContext.routeInvoker().then(() => {
							// Assert:
							expect(routeContext.numNextCalls).to.equal(1);
							expect(routeContext.responses.length).to.equal(1);
							expect(routeContext.redirects.length).to.equal(0);
							expect(routeContext.responses[0]).to.deep.equal({
								formatter: 'ws',
								payload: {
									tree: stateTree
								},
								type: 'stateTree'
							});
						})
					));
			});
		});

		it('returns error for invalid state', () =>
			// Act:
			test.route.prepareExecuteRoute(
				blockRoutes.register,
				'/state/:state/hash/:hash/merkle',
				'get',
				{ state: 15000, hash: '2D3418274BBC250616223C162534B460216AED82C4FA9A87B53083B7BA7A9391' },
				{}, {}, routeContext =>
					test.assert.invokerThrowsError(routeContext.routeInvoker, {
						statusCode: 409,
						message: 'invalid `state` provided'
					})
			));
	});
});
