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

const restrictionsRoutes = require('../../../src/plugins/restrictions/restrictionsRoutes');
const routeResultTypes = require('../../../src/routes/routeResultTypes');
const routeUtils = require('../../../src/routes/routeUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { addresses } = test.sets;

describe('restrictions routes', () => {
	describe('account restrictions', () => {
		describe('get by address', () => {
			test.route.document.addGetDocumentRouteTests(restrictionsRoutes.register, {
				route: '/restrictions/account/:address',
				inputs: {
					valid: {
						object: { address: addresses.valid[0] },
						parsed: [[address.stringToAddress(addresses.valid[0])]],
						printable: addresses.valid[0]
					},
					invalid: {
						object: { address: addresses.invalid },
						error: 'address has an invalid format'
					}
				},
				dbApiName: 'accountRestrictionsByAddresses',
				type: 'accountRestrictions'
			});
		});
	});

	describe('mosaic restrictions', () => {
		const testMosaicId = '0DC67FBE1CAD29E3';
		const testMosaicIdParsed = [0x1CAD29E3, 0x0DC67FBE];
		const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ';

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
					mosaicRestrictionEntry: {
						compositeHash: '',
						entryType: 0,
						mosaicId: '',
						targetAddress: '',
						restrictions: []
					}
				},
				{
					id: '',
					mosaicRestrictionEntry: {
						compositeHash: '',
						entryType: 1,
						mosaicId: '',
						restrictions: []
					}
				}
			],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const dbMosaicRestrictionsFake = sinon.fake(mosaicId =>
			(mosaicId ? Promise.resolve(emptyPageSample) : Promise.resolve(pageSample)));

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
		const db = { mosaicRestrictions: dbMosaicRestrictionsFake };
		restrictionsRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			dbMosaicRestrictionsFake.resetHistory();
		});

		describe('GET', () => {
			const route = mockServer.getRoute('/restrictions/mosaic').get();

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

					expect(dbMosaicRestrictionsFake.calledOnce).to.equal(true);
					expect(dbMosaicRestrictionsFake.firstCall.args[3]).to.deep.equal(pagingBag);
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

			it('returns empty page if no restrictions found', () => {
				// Arrange:
				const req = { params: { mosaicId: testMosaicId } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicRestrictionsFake.calledOnce).to.equal(true);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: emptyPageSample,
						type: routeResultTypes.mosaicRestrictions,
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards mosaicId', () => {
				// Arrange:
				const req = { params: { mosaicId: testMosaicId } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicRestrictionsFake.calledOnce).to.equal(true);
					expect(dbMosaicRestrictionsFake.firstCall.args[0]).to.deep.equal(testMosaicIdParsed);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards entryType', () => {
				// Arrange:
				const req = { params: { entryType: '0' } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicRestrictionsFake.calledOnce).to.equal(true);
					expect(dbMosaicRestrictionsFake.firstCall.args[1]).to.equal(0);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards targetAddress', () => {
				// Arrange:
				const req = { params: { targetAddress: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicRestrictionsFake.calledOnce).to.equal(true);
					expect(dbMosaicRestrictionsFake.firstCall.args[2]).to.deep.equal(address.stringToAddress(testAddress));

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns page with results', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicRestrictionsFake.calledOnce).to.equal(true);
					expect(dbMosaicRestrictionsFake.firstCall.args[0]).to.deep.equal(undefined);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: pageSample,
						type: routeResultTypes.mosaicRestrictions,
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('throws error if mosaicId is invalid', () => {
				// Arrange:
				const req = { params: { mosaicId: '12345' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('mosaicId has an invalid format');
			});

			it('throws error if entryType is invalid', () => {
				// Arrange:
				const req = { params: { entryType: '-1' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('entryType has an invalid format');
			});

			it('throws error if targetAddress is invalid', () => {
				// Arrange:
				const req = { params: { targetAddress: 'AB12345' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('targetAddress has an invalid format');
			});
		});
	});
});
