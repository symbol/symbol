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

import lockHashRoutes from '../../../src/plugins/lockHash/lockHashRoutes.js';
import routeUtils from '../../../src/routes/routeUtils.js';
import test from '../../routes/utils/routeTestUtils.js';
import { expect } from 'chai';
import sinon from 'sinon';
import { Address } from 'symbol-sdk/symbol';

const { MockServer } = test;

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
			(Buffer.from(addresses[0]).equals(new Address(testAddress).bytes)
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
						expect(dbHashLocksFake.firstCall.args[0]).to.deep.equal([new Address(testAddress).bytes]);

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
						expect(dbHashLocksFake.firstCall.args[0]).to.deep.equal([new Address(testAddressNoLocks).bytes]);

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
						expect(dbHashLocksFake.firstCall.args[0]).to.deep.equal([new Address(testAddress).bytes]);

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
		});

		describe('by hash', () => {
			const hashes = ['C54AFD996DF1F52748EBC5B40F8D0DC242A6A661299149F5F96A0C21ECCB653F'];
			const uint64Hashes = hashes.map(routeUtils.namedParserMap.hash256);
			test.route.document.addGetPostDocumentRouteTests(lockHashRoutes.register, {
				routes: { singular: '/lock/hash/:hash', plural: '/lock/hash' },

				inputs: {
					valid: { object: { hash: hashes[0] }, parsed: [uint64Hashes[0]], printable: hashes[0] },
					validMultiple: { object: { hashes }, parsed: uint64Hashes },
					invalid: { object: { hash: '12345' }, error: 'hash has an invalid format' },
					invalidMultiple: {
						object: { hashes: [hashes[0], '12345'] },
						error: 'element in array hashes has an invalid format'
					}
				},

				dbApiName: 'hashLockByHash',
				type: 'hashLockInfo'
			});
		});
	});
});
