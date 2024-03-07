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

const catapult = require('../../../src/catapult-sdk/index');
const metadataRoutes = require('../../../src/plugins/metadata/metadataRoutes');
const routeUtils = require('../../../src/routes/routeUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;

describe('metadata routes', () => {
	describe('metadata', () => {
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
					metadataEntry: {
						compositeHash: 'random1',
						sourceAddress: 1,
						targetAddress: '',
						scopedMetadataKey: '',
						targetId: 1,
						metadataType: 3,
						valueSize: 3,
						value: ''
					}
				},
				{
					id: '',
					metadataEntry: {
						compositeHash: 'random2',
						sourceAddress: 1,
						targetAddress: '',
						scopedMetadataKey: '',
						targetId: 1,
						metadataType: 3,
						valueSize: 3,
						value: ''
					}
				}
			],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const dbMetadataFake = sinon.fake(sourceAddress =>
			(sourceAddress ? Promise.resolve(emptyPageSample) : Promise.resolve(pageSample)));

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
		const db = { metadata: dbMetadataFake };
		metadataRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			dbMetadataFake.resetHistory();
		});

		describe('GET', () => {
			const route = mockServer.getRoute('/metadata').get();

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

					expect(dbMetadataFake.calledOnce).to.equal(true);
					expect(dbMetadataFake.firstCall.args[5]).to.deep.equal(pagingBag);
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

			it('returns empty page if no metadata found', () => {
				// Arrange:
				const req = { params: { sourceAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMetadataFake.calledOnce).to.equal(true);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: emptyPageSample,
						type: 'metadata',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards sourceAddress', () => {
				// Arrange:
				const req = { params: { sourceAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMetadataFake.calledOnce).to.equal(true);
					expect(dbMetadataFake.firstCall.args[0]).to.deep.equal(address.stringToAddress(testAddress));

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards targetAddress', () => {
				// Arrange:
				const req = { params: { targetAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMetadataFake.calledOnce).to.equal(true);
					expect(dbMetadataFake.firstCall.args[1]).to.deep.equal(address.stringToAddress(testAddress));

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards scopedMetadataKey', () => {
				// Arrange:
				const req = { params: { scopedMetadataKey: '0DC67FBE1CAD29E3' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMetadataFake.calledOnce).to.equal(true);
					expect(dbMetadataFake.firstCall.args[2]).to.deep.equal([0x1CAD29E3, 0x0DC67FBE]);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards targetId', () => {
				// Arrange:
				const req = { params: { targetId: '0DC67FBE1CAD29E3' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMetadataFake.calledOnce).to.equal(true);
					expect(dbMetadataFake.firstCall.args[3]).to.deep.equal([0x1CAD29E3, 0x0DC67FBE]);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards metadataType', () => {
				// Arrange:
				const req = { params: { metadataType: '1' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMetadataFake.calledOnce).to.equal(true);
					expect(dbMetadataFake.firstCall.args[4]).to.equal(1);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns page with results', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMetadataFake.calledOnce).to.equal(true);
					expect(dbMetadataFake.firstCall.args[0]).to.deep.equal(undefined);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: pageSample,
						type: 'metadata',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});
	});

	describe('metal', () => {
		const mockServer = new MockServer();
		const db = {
			binDataByMetalId: sinon.stub().resolves({ payload: 'db_payload', text: 'db_text' })
		};
		const services = {
			config: {
				metal: {
					cacheTtl: 300,
					sizeLimit: 10000000
				}
			}
		};
		metadataRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			mockServer.res.setHeader = sinon.spy();
			mockServer.res.write = sinon.spy();
			mockServer.res.end = sinon.spy();
		});

		describe('get by metal id', () => {
			const callRouteAndAssert = async (route, req, shouldCallDb) => {
				await mockServer.callRoute(route, req).then(() => {
					expect(mockServer.res.write.calledOnce).to.equal(true);
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.res.setHeader.calledWithExactly('content-type', 'image/png')).to.equal(true);
					expect(mockServer.res.setHeader
						.calledWithExactly('Content-Disposition', 'attachment; filename="image.png"')).to.equal(true);
					expect(mockServer.res.setHeader.calledWithExactly('Content-MetalText', 'db_text')).to.equal(true);
					expect(db.binDataByMetalId.calledOnce).to.equal(shouldCallDb);
					if (shouldCallDb)
						expect(db.binDataByMetalId.alwaysCalledWith('metal_id')).to.equal(true);
				});
			};

			// Arrange:
			const route = mockServer.getRoute('/metadata/metal/:metalId').get();
			const req = {
				params: {
					metalId: 'metal_id',
					mimeType: 'image/png',
					fileName: 'image.png',
					download: 'true'
				}
			};

			it('returns page with results and uses DB on single call', async () => {
				// Act + Assert:
				callRouteAndAssert(route, req, true);
			});

			it('returns page with results and uses cache on second call', async () => {
				// Arrange:
				// First call
				await mockServer.callRoute(route, req);

				// Reset the spies
				mockServer.res.write.resetHistory();
				mockServer.next.resetHistory();
				mockServer.res.setHeader.resetHistory();
				db.binDataByMetalId.resetHistory();

				// Act & Assert:
				await callRouteAndAssert(route, req, false);
			});
		});
	});
});
