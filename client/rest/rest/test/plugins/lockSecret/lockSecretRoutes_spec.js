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

const lockSecretRoutes = require('../../../src/plugins/lockSecret/lockSecretRoutes');
const routeUtils = require('../../../src/routes/routeUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { convert } = catapult.utils;

describe('lock secret routes', () => {
	describe('secret locks', () => {
		const testAddress = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA';
		const testAddressNoLocks = 'A34B57B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ45AB';
		const testSecret = '5994471ABB01112AFCC18159F6CC74B4F511B99806DA59B3CAF5A9C173CACFC5';

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
						hashAlgorithm: '',
						secret: '',
						recipientAddress: '',
						compositeHash: ''
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
						hashAlgorithm: '',
						secret: '',
						recipientAddress: '',
						compositeHash: ''
					}
				}
			],
			pagination: {
				pageNumber: 1,
				pageSize: 10
			}
		};

		const dbSecretLocksFake = sinon.fake(addresses =>
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
		const db = { secretLocks: dbSecretLocksFake };
		lockSecretRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			dbSecretLocksFake.resetHistory();
		});

		describe('GET', () => {
			const route = mockServer.getRoute('/account/:address/lock/secret').get();

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
						expect(dbSecretLocksFake.calledOnce).to.equal(true);
						expect(dbSecretLocksFake.firstCall.args[2]).to.deep.equal(pagingBag);
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
						expect(dbSecretLocksFake.calledOnce).to.equal(true);
						expect(dbSecretLocksFake.firstCall.args[0]).to.deep.equal([address.stringToAddress(testAddress)]);

						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('forwards secret', () => {
					// Arrange:
					const req = { params: { address: testAddress, secret: testSecret } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbSecretLocksFake.calledOnce).to.equal(true);
						expect(dbSecretLocksFake.firstCall.args[1]).to.deep.equal(convert.hexToUint8(testSecret));

						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('returns empty if no secret locks found', () => {
					// Arrange:
					const req = { params: { address: testAddressNoLocks } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbSecretLocksFake.calledOnce).to.equal(true);
						expect(dbSecretLocksFake.firstCall.args[0]).to.deep.equal([address.stringToAddress(testAddressNoLocks)]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: emptyPageSample,
							type: 'secretLockInfo',
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
						expect(dbSecretLocksFake.calledOnce).to.equal(true);
						expect(dbSecretLocksFake.firstCall.args[0]).to.deep.equal([address.stringToAddress(testAddress)]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: pageSample,
							type: 'secretLockInfo',
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
	});
});
