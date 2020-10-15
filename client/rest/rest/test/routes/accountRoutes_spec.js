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

const { MockServer } = require('./utils/routeTestUtils');
const { test } = require('./utils/routeTestUtils');
const accountRoutes = require('../../src/routes/accountRoutes');
const routeResultTypes = require('../../src/routes/routeResultTypes');
const routeUtils = require('../../src/routes/routeUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { convert } = catapult.utils;

describe('account routes', () => {
	const testAddress = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDA';
	const testPublicKey = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';

	describe('GET', () => {
		describe('accounts', () => {
			const testMosaicId = 'ABCDEF0123456789';

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
						account: {
							address: '',
							addressHeight: '',
							publicKey: '',
							publicKeyHeight: '',
							supplementalPublicKeys: {},
							importance: '',
							importanceHeight: '',
							activityBuckets: [],
							mosaics: []
						}
					},
					{
						id: 'random2',
						account: {
							address: '',
							addressHeight: '',
							publicKey: '',
							publicKeyHeight: '',
							supplementalPublicKeys: {},
							importance: '',
							importanceHeight: '',
							activityBuckets: [],
							mosaics: []
						}
					}
				],
				pagination: {
					pageNumber: 1,
					pageSize: 10
				}
			};

			const dbAccountsFake = sinon.fake(accountAddress =>
				(accountAddress ? Promise.resolve(emptyPageSample) : Promise.resolve(pageSample)));

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
			const db = { accounts: dbAccountsFake };
			accountRoutes.register(mockServer.server, db, services);

			beforeEach(() => {
				mockServer.resetStats();
				dbAccountsFake.resetHistory();
			});

			const route = mockServer.getRoute('/accounts').get();

			it('parses and forwards paging options', () => {
				// Arrange:
				const pagingBag = 'fakePagingBagObject';
				const paginationParser = sinon.stub(routeUtils, 'parsePaginationArguments').returns(pagingBag);
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(paginationParser.firstCall.args[0]).to.deep.equal(req.params);
					expect(paginationParser.firstCall.args[2]).to.deep.equal({
						id: 'objectId',
						balance: 'uint64'
					});

					expect(dbAccountsFake.calledOnce).to.equal(true);
					expect(dbAccountsFake.firstCall.args[2]).to.deep.equal(pagingBag);
					paginationParser.restore();
				});
			});

			it('allowed sort fields are taken into account', () => {
				// Arrange:
				const paginationParserSpy = sinon.spy(routeUtils, 'parsePaginationArguments');
				const expectedAllowedSortFields = { id: 'objectId', balance: 'uint64' };

				// Act:
				return mockServer.callRoute(route, { params: {} }).then(() => {
					// Assert:
					expect(paginationParserSpy.calledOnce).to.equal(true);
					expect(paginationParserSpy.firstCall.args[2]).to.deep.equal(expectedAllowedSortFields);
					paginationParserSpy.restore();
				});
			});

			it('returns empty page if no accounts found', () => {
				// Arrange:
				const req = { params: { address: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbAccountsFake.calledOnce).to.equal(true);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: emptyPageSample,
						type: routeResultTypes.account,
						structure: 'page'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards query without address if not provided', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbAccountsFake.calledOnce).to.equal(true);
					expect(dbAccountsFake.firstCall.args[0]).to.deep.equal(undefined);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards query without mosaicId if not provided', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbAccountsFake.calledOnce).to.equal(true);
					expect(dbAccountsFake.firstCall.args[1]).to.deep.equal(undefined);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards address', () => {
				// Arrange:
				const req = { params: { address: testAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbAccountsFake.calledOnce).to.equal(true);
					expect(dbAccountsFake.firstCall.args[0]).to.deep.equal(address.stringToAddress(testAddress));

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('forwards mosaicId', () => {
				// Arrange:
				const req = { params: { mosaicId: testMosaicId } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbAccountsFake.calledOnce).to.equal(true);
					expect(dbAccountsFake.firstCall.args[1]).to.deep.equal([0x23456789, 0xABCDEF01]);

					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns page with results', () => {
				// Arrange:
				const req = { params: {} };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbAccountsFake.calledOnce).to.equal(true);
					expect(dbAccountsFake.firstCall.args[0]).to.deep.equal(undefined);
					expect(dbAccountsFake.firstCall.args[1]).to.deep.equal(undefined);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: pageSample,
						type: routeResultTypes.account,
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

			it('throws error if mosaicId is invalid', () => {
				// Arrange:
				const req = { params: { mosaicId: 'AB12345' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('mosaicId has an invalid format');
			});

			it('throws error if there is no mosaicId when sorting by balance', () => {
				// Arrange:
				const req = { params: { orderBy: 'balance' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('mosaicId must be provided when sorting by balance');
			});
		});

		describe('by accountId', () => {
			describe('by address', () => {
				test.route.document.addGetDocumentRouteTests(accountRoutes.register, {
					route: '/accounts/:accountId',
					inputs: {
						valid: {
							object: { accountId: testAddress },
							parsed: [[{ address: address.stringToAddress(testAddress) }]],
							printable: testAddress
						},
						invalid: {
							object: { accountId: '12345' },
							error: 'accountId has an invalid format'
						}
					},
					dbApiName: 'accountsByIds',
					type: routeResultTypes.account
				});
			});

			describe('by publicKey', () => {
				test.route.document.addGetDocumentRouteTests(accountRoutes.register, {
					route: '/accounts/:accountId',
					inputs: {
						valid: {
							object: { accountId: testPublicKey },
							parsed: [[{ publicKey: convert.hexToUint8(testPublicKey) }]],
							printable: testPublicKey
						},
						invalid: {
							object: { accountId: '12345' },
							error: 'accountId has an invalid format'
						}
					},
					dbApiName: 'accountsByIds',
					type: routeResultTypes.account
				});
			});
		});
	});

	describe('POST', () => {
		describe('accounts', () => {
			const fakeAccounts = [{ id: '', account: { address: '' } }];
			const dbAccountsByIds = sinon.fake.resolves(fakeAccounts);
			const mockServer = new MockServer();
			const db = { accountsByIds: dbAccountsByIds };
			accountRoutes.register(mockServer.server, db, {});

			const route = mockServer.getRoute('/accounts').post();

			beforeEach(() => {
				mockServer.resetStats();
				dbAccountsByIds.resetHistory();
			});

			it('throws if both publicKeys and addresses are provided', () => {
				// Arrange
				const req = { params: { addresses: [], publicKeys: [] } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('publicKeys and addresses cannot both be provided');
			});

			const runParseArgumentAsArrayParamTest = (paramValues, paramName, parserName) => {
				// Arrange
				const req = { params: { [paramName]: paramValues } };
				const parseArgumentsAsArraySpy = sinon.spy(routeUtils, 'parseArgumentAsArray');

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(parseArgumentsAsArraySpy.calledOnceWith(
						{ [paramName]: paramValues },
						paramName,
						parserName
					)).to.equal(true);
					parseArgumentsAsArraySpy.restore();
				});
			};

			it('calls parseArgumentAsArray with correct parser for addresses', () =>
				runParseArgumentAsArrayParamTest([testAddress, testAddress], 'addresses', 'address'));

			it('calls parseArgumentAsArray with correct parser for public keys', () =>
				runParseArgumentAsArrayParamTest([testPublicKey, testPublicKey], 'publicKeys', 'publicKey'));

			describe('calls correct accounts retriever with correct params', () => {
				it('adress params', () => {
					// Arrange
					const req = { params: { addresses: [testAddress] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbAccountsByIds.calledOnce).to.equal(true);
						expect(dbAccountsByIds.firstCall.args[0]).to.deep.equal([{ address: address.stringToAddress(testAddress) }]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakeAccounts,
							type: routeResultTypes.account
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('publicKey params', () => {
					// Arrange
					const req = { params: { publicKeys: [testPublicKey] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbAccountsByIds.calledOnce).to.equal(true);
						expect(dbAccountsByIds.firstCall.args[0]).to.deep.equal([{ publicKey: convert.hexToUint8(testPublicKey) }]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakeAccounts,
							type: routeResultTypes.account
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			});
		});
	});
});
