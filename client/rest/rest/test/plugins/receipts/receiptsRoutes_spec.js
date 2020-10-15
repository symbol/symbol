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

const receiptsRoutes = require('../../../src/plugins/receipts/receiptsRoutes');
const routeUtils = require('../../../src/routes/routeUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;

describe('receipts routes', () => {
	describe('transaction statements', () => {
		const testAddress = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA';

		const emptyPageSample = {
			data: [],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const pageSample = {
			data: [
				{
					id: '',
					statement: {
						height: '',
						source: {
							primaryId: 0,
							secondaryId: 0
						},
						receipts: [
							{
								version: 1,
								type: 8515,
								targetAddress: '',
								mosaicId: '',
								amount: ''
							},
							{
								version: 1,
								type: 8515,
								targetAddress: '',
								mosaicId: '',
								amount: ''
							},
							{
								version: 1,
								type: 20803,
								mosaicId: '',
								amount: ''
							}
						]
					}
				},
				{
					id: '',
					statement: {
						height: '',
						source: {
							primaryId: 0,
							secondaryId: 0
						},
						receipts: [
							{
								version: 1,
								type: 8515,
								targetAddress: '',
								mosaicId: '',
								amount: ''
							},
							{
								version: 1,
								type: 8515,
								targetAddress: '',
								mosaicId: '',
								amount: ''
							},
							{
								version: 1,
								type: 20803,
								mosaicId: '',
								amount: ''
							}
						]
					}
				}
			],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const dbTransactionStatementsFake = sinon.fake(filters => {
			if (filters.height && 666 === filters.height[0])
				return Promise.resolve(emptyPageSample);

			return Promise.resolve(pageSample);
		});

		const services = {
			config: {
				pageSize: {
					min: 10,
					max: 100,
					default: 20
				}
			}
		};

		const mockServer = new MockServer();
		const db = { transactionStatements: dbTransactionStatementsFake };
		receiptsRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			dbTransactionStatementsFake.resetHistory();
		});

		describe('GET', () => {
			const route = mockServer.getRoute('/statements/transaction').get();

			it('parses and forwards paging options', () => {
				// Arrange:
				const pagingBag = 'fakePagingBagObject';
				const paginationParser = sinon.stub(routeUtils, 'parsePaginationArguments').returns(pagingBag);
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(paginationParser.firstCall.args[0]).to.deep.equal(req.params);
					expect(paginationParser.firstCall.args[2]).to.deep.equal({ id: 'objectId' });

					expect(dbTransactionStatementsFake.calledOnce).to.equal(true);
					expect(dbTransactionStatementsFake.firstCall.args[1]).to.deep.equal(pagingBag);
					paginationParser.restore();
				});
			});

			it('allowed sort fields are taken into account', () => {
				// Arrange:
				const paginationParserSpy = sinon.spy(routeUtils, 'parsePaginationArguments');
				const expectedAllowedSortFields = { id: 'objectId' };

				// Act:
				return mockServer.callRoute(route, { params: {} }).then(() => {
					// Assert:
					expect(paginationParserSpy.calledOnce).to.equal(true);
					expect(paginationParserSpy.firstCall.args[2]).to.deep.equal(expectedAllowedSortFields);
					paginationParserSpy.restore();
				});
			});

			it('returns empty page if no statements found', () => {
				// Arrange:
				const req = { params: { height: '666' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbTransactionStatementsFake.calledOnce).to.equal(true);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: emptyPageSample,
						type: 'transactionStatement',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards height', () => {
				// Arrange:
				const req = { params: { height: '123' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbTransactionStatementsFake.calledOnce).to.equal(true);
					expect(dbTransactionStatementsFake.firstCall.args[0].height).to.deep.equal([123, 0]);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			describe('forwards receiptType', () => {
				it('one element', () => {
					// Arrange:
					const req = { params: { receiptType: '456' } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbTransactionStatementsFake.calledOnce).to.equal(true);
						expect(dbTransactionStatementsFake.firstCall.args[0].receiptType).to.deep.equal([456]);

						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('multiple elements', () => {
					// Arrange:
					const req = { params: { receiptType: ['456', '457'] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbTransactionStatementsFake.calledOnce).to.equal(true);
						expect(dbTransactionStatementsFake.firstCall.args[0].receiptType).to.deep.equal([456, 457]);

						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			});

			it('forwards recipientAddress', () => {
				// Arrange:
				const req = { params: { recipientAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbTransactionStatementsFake.calledOnce).to.equal(true);
					expect(dbTransactionStatementsFake.firstCall.args[0].recipientAddress)
						.to.deep.equal(address.stringToAddress(testAddress));

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards senderAddress', () => {
				// Arrange:
				const req = { params: { senderAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbTransactionStatementsFake.calledOnce).to.equal(true);
					expect(dbTransactionStatementsFake.firstCall.args[0].senderAddress).to.deep.equal(address.stringToAddress(testAddress));

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards targetAddress', () => {
				// Arrange:
				const req = { params: { targetAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbTransactionStatementsFake.calledOnce).to.equal(true);
					expect(dbTransactionStatementsFake.firstCall.args[0].targetAddress).to.deep.equal(address.stringToAddress(testAddress));

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards artifactId', () => {
				// Arrange:
				const req = { params: { artifactId: '0DC67FBE1CAD29E3' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbTransactionStatementsFake.calledOnce).to.equal(true);
					expect(dbTransactionStatementsFake.firstCall.args[0].artifactId).to.deep.equal([0x1CAD29E3, 0x0DC67FBE]);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns page with statement results', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbTransactionStatementsFake.calledOnce).to.equal(true);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: pageSample,
						type: 'transactionStatement',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});
	});

	describe('artifact resolution statements', () => {
		const emptyPageSample = {
			data: [],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const pageSample = {
			data: [
				{
					id: '',
					statement: {
						height: '',
						unresolved: '',
						resolutionEntries: [
							{
								source: {
									primaryId: 2,
									secondaryId: 0
								},
								resolved: ''
							}
						]
					}
				},
				{
					id: '',
					statement: {
						height: '',
						unresolved: '',
						resolutionEntries: [
							{
								source: {
									primaryId: 2,
									secondaryId: 0
								},
								resolved: ''
							}
						]
					}
				}
			],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const dbArtifactStatementsFake = sinon.fake(height => {
			if (height && 666 === height[0])
				return Promise.resolve(emptyPageSample);

			return Promise.resolve(pageSample);
		});

		const services = {
			config: {
				pageSize: {
					min: 10,
					max: 100,
					default: 20
				}
			}
		};

		const mockServer = new MockServer();
		const db = { artifactStatements: dbArtifactStatementsFake };
		receiptsRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			dbArtifactStatementsFake.resetHistory();
		});

		describe('GET', () => {
			const route = mockServer.getRoute('/statements/resolutions/:artifact').get();

			it('parses and forwards paging options', () => {
				// Arrange:
				const pagingBag = 'fakePagingBagObject';
				const paginationParser = sinon.stub(routeUtils, 'parsePaginationArguments').returns(pagingBag);
				const req = { params: { artifact: 'address' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(paginationParser.firstCall.args[0]).to.deep.equal(req.params);
					expect(paginationParser.firstCall.args[2]).to.deep.equal({ id: 'objectId' });

					expect(dbArtifactStatementsFake.calledOnce).to.equal(true);
					expect(dbArtifactStatementsFake.firstCall.args[2]).to.deep.equal(pagingBag);
					paginationParser.restore();
				});
			});

			it('allowed sort fields are taken into account', () => {
				// Arrange:
				const paginationParserSpy = sinon.spy(routeUtils, 'parsePaginationArguments');
				const expectedAllowedSortFields = { id: 'objectId' };

				// Act:
				return mockServer.callRoute(route, { params: { artifact: 'address' } }).then(() => {
					// Assert:
					expect(paginationParserSpy.calledOnce).to.equal(true);
					expect(paginationParserSpy.firstCall.args[2]).to.deep.equal(expectedAllowedSortFields);
					paginationParserSpy.restore();
				});
			});

			it('returns empty address statements page if no statements found', () => {
				// Arrange:
				const req = { params: { artifact: 'address', height: '666' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbArtifactStatementsFake.calledOnce).to.equal(true);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: emptyPageSample,
						type: 'addressResolutionStatement',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns empty mosaic statements page if no statements found', () => {
				// Arrange:
				const req = { params: { artifact: 'mosaic', height: '666' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbArtifactStatementsFake.calledOnce).to.equal(true);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: emptyPageSample,
						type: 'mosaicResolutionStatement',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards height', () => {
				// Arrange:
				const req = { params: { artifact: 'address', height: '123' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbArtifactStatementsFake.calledOnce).to.equal(true);
					expect(dbArtifactStatementsFake.firstCall.args[0]).to.deep.equal([123, 0]);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards address artifact', () => {
				// Arrange:
				const req = { params: { artifact: 'address' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbArtifactStatementsFake.calledOnce).to.equal(true);
					expect(dbArtifactStatementsFake.firstCall.args[1]).to.equal('address');

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards mosaic artifact', () => {
				// Arrange:
				const req = { params: { artifact: 'mosaic' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbArtifactStatementsFake.calledOnce).to.equal(true);
					expect(dbArtifactStatementsFake.firstCall.args[1]).to.equal('mosaic');

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('fails for invalid artifact', () => {
				// Arrange:
				const req = { params: { artifact: 'namespace' } };

				// Act:
				mockServer.callRoute(route, req);

				// Assert:
				expect(mockServer.next.calledOnce).to.equal(true);
				expect(mockServer.next.firstCall.args[0].statusCode).to.equal(404);
			});

			it('fails if no artifact provided', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				mockServer.callRoute(route, req);

				// Assert:
				expect(mockServer.next.calledOnce).to.equal(true);
				expect(mockServer.next.firstCall.args[0].statusCode).to.equal(404);
			});

			it('returns page with results', () => {
				// Arrange:
				const req = { params: { artifact: 'address' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbArtifactStatementsFake.calledOnce).to.equal(true);
					expect(dbArtifactStatementsFake.firstCall.args[0]).to.deep.equal(undefined);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: pageSample,
						type: 'addressResolutionStatement',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});
	});

	describe('get receipts merkle path', () => {
		it('calls blockRouteMerkleProcessor with correct params', () => {
			// Arrange:
			const mockServer = new MockServer();
			const blockRouteMerkleProcessorSpy = sinon.spy(routeUtils, 'blockRouteMerkleProcessor');

			// Act:
			receiptsRoutes.register(mockServer.server, {}, {});

			// Assert:
			expect(blockRouteMerkleProcessorSpy.calledOnce).to.equal(true);
			expect(blockRouteMerkleProcessorSpy.firstCall.args[1]).to.equal('statementsCount');
			expect(blockRouteMerkleProcessorSpy.firstCall.args[2]).to.equal('statementMerkleTree');
			blockRouteMerkleProcessorSpy.restore();
		});
	});
});
