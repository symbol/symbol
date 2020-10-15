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

const lockHashRoutes = require('../../../src/plugins/lockHash/lockHashRoutes');
const routeUtils = require('../../../src/routes/routeUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { convert } = catapult.utils;

describe('lock hash routes', () => {
	describe('hash locks', () => {
		const testAddress = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA';
		const testAddressNoLocks = 'A34B57B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ45AB';

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
					id: 'random1',
					lock: {
						ownerAddress: '',
						mosaicId: '',
						amount: '',
						endHeight: '',
						status: '',
						hash: ''
					}
				},
				{
					id: 'random2',
					lock: {
						ownerAddress: '',
						mosaicId: '',
						amount: '',
						endHeight: '',
						status: '',
						hash: ''
					}
				}
			],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const dbHashLocksFake = sinon.fake(addresses =>
			(Buffer.from(addresses[0]).equals(Buffer.from(address.stringToAddress(testAddress)))
				? Promise.resolve(pageSample)
				: Promise.resolve(emptyPageSample)));

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
		const db = { hashLocks: dbHashLocksFake };
		lockHashRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			dbHashLocksFake.resetHistory();
		});

		describe('GET', () => {
			const route = mockServer.getRoute('/account/:address/lock/hash').get();

			describe('by address', () => {
				it('parses and forwards paging options', () => {
					// Arrange:
					const pagingBag = 'fakePagingBagObject';
					const paginationParser = sinon.stub(routeUtils, 'parsePaginationArguments').returns(pagingBag);
					const req = { params: { address: testAddress } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(paginationParser.firstCall.args[0]).to.deep.equal(req.params);
						expect(paginationParser.firstCall.args[2]).to.deep.equal({ id: 'objectId' });
						expect(dbHashLocksFake.calledOnce).to.equal(true);
						expect(dbHashLocksFake.firstCall.args[1]).to.deep.equal(pagingBag);
						paginationParser.restore();
					});
				});

				it('allowed sort fields are taken into account', () => {
					// Arrange:
					const paginationParserSpy = sinon.spy(routeUtils, 'parsePaginationArguments');
					const expectedAllowedSortFields = { id: 'objectId' };

					// Act:
					return mockServer.callRoute(route, { params: { address: testAddress } }).then(() => {
						// Assert:
						expect(paginationParserSpy.calledOnce).to.equal(true);
						expect(paginationParserSpy.firstCall.args[2]).to.deep.equal(expectedAllowedSortFields);
						paginationParserSpy.restore();
					});
				});

				it('forwards address', () => {
					// Arrange:
					const req = { params: { address: testAddress } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbHashLocksFake.calledOnce).to.equal(true);
						expect(dbHashLocksFake.firstCall.args[0]).to.deep.equal([address.stringToAddress(testAddress)]);

						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('returns empty if no hash locks found', () => {
					// Arrange:
					const req = { params: { address: testAddressNoLocks } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbHashLocksFake.calledOnce).to.equal(true);
						expect(dbHashLocksFake.firstCall.args[0]).to.deep.equal([address.stringToAddress(testAddressNoLocks)]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: emptyPageSample,
							type: 'hashLockInfo',
							structure: 'page'
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('returns page with results', () => {
					// Arrange:
					const req = { params: { address: testAddress } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbHashLocksFake.calledOnce).to.equal(true);
						expect(dbHashLocksFake.firstCall.args[0]).to.deep.equal([address.stringToAddress(testAddress)]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: pageSample,
							type: 'hashLockInfo',
							structure: 'page'
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('throws error if address is invalid', () => {
					// Arrange:
					const req = { params: { address: 'AB12345' } };

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('address has an invalid format');
				});
			});

			describe('by hash', () => {
				const hash = 'C54AFD996DF1F52748EBC5B40F8D0DC242A6A661299149F5F96A0C21ECCB653F';
				test.route.document.addGetDocumentRouteTests(lockHashRoutes.register, {
					route: '/lock/hash/:hash',
					inputs: {
						valid: { object: { hash }, parsed: [convert.hexToUint8(hash)], printable: hash },
						invalid: {
							object: { hash: '12345' },
							error: 'hash has an invalid format'
						}
					},
					dbApiName: 'hashLockByHash',
					type: 'hashLockInfo'
				});
			});
		});
	});
});
