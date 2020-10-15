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

const mosaicRoutes = require('../../../src/plugins/mosaic/mosaicRoutes');
const routeUtils = require('../../../src/routes/routeUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;

describe('mosaic routes', () => {
	describe('mosaics', () => {
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
					mosaic: {
						id: 'random1',
						supply: 1,
						startHeight: '',
						ownerAddress: '',
						revision: 1,
						flags: 3,
						divisibility: 3,
						duration: ''
					}
				},
				{
					id: '',
					mosaic: {
						id: 'random2',
						supply: 1,
						startHeight: '',
						ownerAddress: '',
						revision: 1,
						flags: 3,
						divisibility: 3,
						duration: ''
					}
				}
			],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const dbMosaicsFake = sinon.fake(ownerAddress =>
			(ownerAddress ? Promise.resolve(emptyPageSample) : Promise.resolve(pageSample)));

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
		const db = { mosaics: dbMosaicsFake };
		mosaicRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			dbMosaicsFake.resetHistory();
		});

		describe('GET', () => {
			const route = mockServer.getRoute('/mosaics').get();

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

					expect(dbMosaicsFake.calledOnce).to.equal(true);
					expect(dbMosaicsFake.firstCall.args[1]).to.deep.equal(pagingBag);
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

			it('returns empty page if no mosaics found', () => {
				// Arrange:
				const req = { params: { ownerAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsFake.calledOnce).to.equal(true);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: emptyPageSample,
						type: 'mosaicDescriptor',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards query without ownerAddress if not provided', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsFake.calledOnce).to.equal(true);
					expect(dbMosaicsFake.firstCall.args[0]).to.deep.equal(undefined);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards ownerAddress', () => {
				// Arrange:
				const req = { params: { ownerAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsFake.calledOnce).to.equal(true);
					expect(dbMosaicsFake.firstCall.args[0]).to.deep.equal(address.stringToAddress(testAddress));

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns page with results', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsFake.calledOnce).to.equal(true);
					expect(dbMosaicsFake.firstCall.args[0]).to.deep.equal(undefined);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: pageSample,
						type: 'mosaicDescriptor',
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('throws error if ownerAddress is invalid', () => {
				// Arrange:
				const req = { params: { ownerAddress: 'AB12345' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('ownerAddress has an invalid format');
			});
		});
	});

	describe('mosaics by id', () => {
		const mosaicIds = ['1234567890ABCDEF', 'ABCDEF0123456789'];
		const uint64MosaicIds = [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]];
		const errorMessage = 'has an invalid format';
		test.route.document.addGetPostDocumentRouteTests(mosaicRoutes.register, {
			routes: { singular: '/mosaics/:mosaicId', plural: '/mosaics' },
			inputs: {
				valid: { object: { mosaicId: mosaicIds[0] }, parsed: [uint64MosaicIds[0]], printable: mosaicIds[0] },
				validMultiple: { object: { mosaicIds }, parsed: uint64MosaicIds },
				invalid: { object: { mosaicId: '12345' }, error: `mosaicId ${errorMessage}` },
				invalidMultiple: {
					object: { mosaicIds: [mosaicIds[0], '12345', mosaicIds[1]] },
					error: `element in array mosaicIds ${errorMessage}`
				}
			},
			dbApiName: 'mosaicsByIds',
			type: 'mosaicDescriptor'
		});
	});
});
