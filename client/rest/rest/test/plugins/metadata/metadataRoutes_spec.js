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

const metadataRoutes = require('../../../src/plugins/metadata/metadataRoutes');
const routeUtils = require('../../../src/routes/routeUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
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
});
