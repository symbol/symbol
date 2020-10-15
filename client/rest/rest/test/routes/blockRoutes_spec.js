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

const { MockServer, test } = require('./utils/routeTestUtils');
const { convertToLong } = require('../../src/db/dbUtils');
const blockRoutes = require('../../src/routes/blockRoutes');
const routeResultTypes = require('../../src/routes/routeResultTypes');
const routeUtils = require('../../src/routes/routeUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { PacketType, StatePathPacketTypes } = catapult.packet;
const { convert } = catapult.utils;

describe('block routes', () => {
	const addChainStatisticToDb = db => { db.chainStatisticCurrent = () => Promise.resolve({ height: convertToLong(10) }); };
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

	describe('blocks', () => {
		describe('get', () => {
			const testPublickeyString = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';
			const testPublickey = convert.hexToUint8(testPublickeyString);

			const testAddressString = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ';
			const testAddress = address.stringToAddress(testAddressString);

			const fakeBlock = { id: 0, meta: { transactionsCount: 0 }, block: { type: 33091 } };
			const fakePaginatedBlock = {
				data: [fakeBlock],
				pagination: {
					pageNumber: 1,
					pageSize: 10
				}
			};
			const dbBlocksFake = sinon.fake.resolves(fakePaginatedBlock);

			const mockServer = new MockServer();
			const db = { blocks: dbBlocksFake };
			const services = {
				config: {
					pageSize: {
						min: 10,
						max: 100,
						default: 20
					}
				}
			};
			blockRoutes.register(mockServer.server, db, services);

			const route = mockServer.getRoute('/blocks').get();

			beforeEach(() => {
				mockServer.resetStats();
				dbBlocksFake.resetHistory();
			});

			it('returns correct structure with blocks', () => {
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: fakePaginatedBlock,
						type: routeResultTypes.block,
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards signerPublicKey filter', () =>
				mockServer.callRoute(route, { params: { signerPublicKey: testPublickeyString } }).then(() => {
					expect(dbBlocksFake.firstCall.args[0]).to.deep.equal(testPublickey);
					expect(dbBlocksFake.firstCall.args[1]).to.deep.equal(undefined);
				}));

			it('forwards beneficiaryAddress filter', () =>
				mockServer.callRoute(route, { params: { beneficiaryAddress: testAddressString } }).then(() => {
					expect(dbBlocksFake.firstCall.args[0]).to.deep.equal(undefined);
					expect(dbBlocksFake.firstCall.args[1]).to.deep.equal(testAddress);
				}));

			describe('parses paging', () => {
				it('parses and forwards paging options', () => {
					// Arrange:
					const pagingBag = 'fakePagingBagObject';
					const paginationParser = sinon.stub(routeUtils, 'parsePaginationArguments').returns(pagingBag);

					// Act:
					const req = { params: {} };
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(paginationParser.firstCall.args[0]).to.deep.equal(req.params);
						expect(paginationParser.firstCall.args[2]).to.deep.equal({ id: 'objectId', height: 'uint64' });

						expect(dbBlocksFake.calledOnce).to.equal(true);
						expect(dbBlocksFake.firstCall.args[2]).to.deep.equal(pagingBag);
						paginationParser.restore();
					});
				});

				it('allowed sort fields are taken into account', () => {
					// Arrange:
					const paginationParserSpy = sinon.spy(routeUtils, 'parsePaginationArguments');
					const expectedAllowedSortFields = {
						id: 'objectId',
						height: 'uint64'
					};

					// Act:
					return mockServer.callRoute(route, { params: {} }).then(() => {
						// Assert:
						expect(paginationParserSpy.calledOnce).to.equal(true);
						expect(paginationParserSpy.firstCall.args[2]).to.deep.equal(expectedAllowedSortFields);
						paginationParserSpy.restore();
					});
				});
			});
		});
	});

	describe('block at height', () => {
		const builder = test.route.document.prepareGetDocumentRouteTests(blockRoutes.register, {
			route: '/blocks/:height',
			dbApiName: 'blockAtHeight',
			type: 'blockHeaderWithMetadata',
			extendDb: addChainStatisticToDb,
			config: routeConfig
		});
		builder.addDefault({
			valid: { object: { height: '3' }, parsed: [[3, 0]], printable: '3' },
			invalid: { object: { height: '10A' }, error: 'height has an invalid format' }
		});
		builder.addNotFoundInputTest({ object: { height: '11' }, parsed: [[11, 0]], printable: '11' }, 'chain height is too small');
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
			expect(blockRouteMerkleProcessorSpy.firstCall.args[1]).to.equal('transactionsCount');
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
