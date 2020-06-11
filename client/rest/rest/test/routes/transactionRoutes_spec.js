/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
const routeResultTypes = require('../../src/routes/routeResultTypes');
const routeUtils = require('../../src/routes/routeUtils');
const transactionRoutes = require('../../src/routes/transactionRoutes');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { convert } = catapult.utils;

describe('transaction routes', () => {
	describe('transaction', () => {
		describe('PUT transaction', () => {
			test.route.packet.addPutPacketRouteTests(transactionRoutes.register, {
				routeName: '/transactions',
				packetType: '9',
				inputs: {
					valid: {
						params: { payload: '123456' },
						parsed: Buffer.of(
							0x0B, 0x00, 0x00, 0x00, // size (header)
							0x09, 0x00, 0x00, 0x00, // type (header)
							0x12, 0x34, 0x56 // payload
						)
					},
					invalid: {
						params: { payload: '1234S6' },
						error: { key: 'payload' }
					}
				}
			});
		});

		describe('get', () => {
			const addGetPostTests = (dbApiName, key, ids, parsedIds) => {
				const errorMessage = 'has an invalid format';
				test.route.document.addGetPostDocumentRouteTests(transactionRoutes.register, {
					routes: { singular: '/transactions/:transactionId', plural: '/transactions' },
					inputs: {
						valid: { object: { transactionId: ids[0] }, parsed: [parsedIds[0]], printable: ids[0] },
						validMultiple: { object: { transactionIds: ids }, parsed: parsedIds },
						invalid: { object: { transactionId: '12345' }, error: `transactionId ${errorMessage}` },
						invalidMultiple: {
							object: { transactionIds: ['12345', ids[0], ids[1]] },
							error: `element in array transactionIds ${errorMessage}`
						}
					},
					dbApiName,
					type: routeResultTypes.transaction
				});
			};

			const addHomogeneousCheck = (validIds, invalidId) => {
				it('does not support lookup of heterogenous ids', () => {
					// Arrange:
					const keyGroups = [];
					const db = test.setup.createCapturingDb('transactionsByIds', keyGroups, [{ value: 'this is nonsense' }]);

					// Act:
					const registerRoutes = transactionRoutes.register;
					const ids = [validIds[0], validIds[1], invalidId, validIds[2]];
					const errorMessage = 'element in array transactionIds has an invalid format';
					return test.route.executeThrows(
						registerRoutes,
						'/transactions',
						'post',
						{ transactionIds: ids },
						db,
						undefined,
						errorMessage,
						409
					);
				});
			};

			const Valid_Object_Ids = ['00112233445566778899AABB', 'CCDDEEFF0011223344556677', '8899AABBCCDDEEFF00112233'];
			const Valid_Transaction_Hashes = [
				'00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF',
				'112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00',
				'2233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF0011'
			];

			describe('objectId', () => {
				addGetPostTests('transactionsByIds', 'transactionId', Valid_Object_Ids, Valid_Object_Ids);
				addHomogeneousCheck(Valid_Object_Ids, Valid_Transaction_Hashes[0]);
			});

			describe('transactionHash', () => {
				addGetPostTests(
					'transactionsByHashes',
					'transactionId',
					Valid_Transaction_Hashes,
					Valid_Transaction_Hashes.map(convert.hexToUint8)
				);
				addHomogeneousCheck(Valid_Transaction_Hashes, Valid_Object_Ids[0]);
			});
		});
	});

	describe('transactions', () => {
		describe('get', () => {
			const testAddressString = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ';
			const testAddress = address.stringToAddress(testAddressString);

			const testPublickeyString = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';
			const testPublickey = convert.hexToUint8(testPublickeyString);

			const fakeTransaction = { meta: { addresses: [] }, transaction: { type: 12345 } };
			const fakePaginatedTransaction = {
				data: [fakeTransaction],
				pagination: {
					pageNumber: 1,
					pageSize: 10,
					totalEntries: 1,
					totalPages: 1
				}
			};
			const dbTransactionsFake = sinon.fake.resolves(fakePaginatedTransaction);

			const mockServer = new MockServer();
			const db = { transactions: dbTransactionsFake };
			const services = {
				config: {
					pageSize: {
						min: 10,
						max: 100,
						default: 20
					}
				}
			};
			transactionRoutes.register(mockServer.server, db, services);

			const route = mockServer.getRoute('/transactions').get();

			beforeEach(() => {
				mockServer.resetStats();
				dbTransactionsFake.resetHistory();
			});

			describe('returns transactions', () => {
				it('returns correct structure with transactions', () => {
					const req = {
						params: {}
					};

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakePaginatedTransaction,
							type: routeResultTypes.transaction,
							structure: 'page'
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			});

			describe('parses filters', () => {
				const runParseFilterTest = (filter, param, value) => {
					it(filter, () => {
						const req = { params: { [filter]: param } };

						const expectedResult = {
							address: undefined,
							height: undefined,
							recipientAddress: undefined,
							signerPublicKey: undefined,
							group: undefined,
							embedded: undefined,
							transactionTypes: undefined
						};

						expectedResult[filter] = value;

						// Act + Assert
						return mockServer.callRoute(route, req).then(() => {
							expect(dbTransactionsFake.firstCall.args[0]).to.deep.equal(expectedResult);
						});
					});
				};

				const testCases = [
					{ filter: 'height', param: '15', value: 15 },
					{ filter: 'address', param: testAddressString, value: testAddress },
					{ filter: 'signerPublicKey', param: testPublickeyString, value: testPublickey },
					{ filter: 'recipientAddress', param: testAddressString, value: testAddress },
					{ filter: 'embedded', param: 'true', value: true },
					{ filter: 'group', param: 'confirmed', value: 'confirmed' }
				];

				testCases.forEach(testCase => {
					runParseFilterTest(testCase.filter, testCase.param, testCase.value);
				});

				it('transactionTypes', () => {
					const req = { params: { type: ['1', '5', '25'] } };

					// Act + Assert
					return mockServer.callRoute(route, req).then(() => {
						expect(dbTransactionsFake.firstCall.args[0]).to.deep.equal({
							address: undefined,
							height: undefined,
							recipientAddress: undefined,
							signerPublicKey: undefined,
							group: undefined,
							embedded: undefined,
							transactionTypes: [1, 5, 25]
						});
					});
				});
			});

			describe('parses options', () => {
				it('parses and forwards paging options', () => {
					// Arrange:
					const pagingBag = 'fakePagingBagObject';
					const paginationParser = sinon.stub(routeUtils, 'parsePaginationArguments').returns(pagingBag);

					// Act:
					return mockServer.callRoute(route, { params: {} }).then(() => {
						// Assert:
						expect(dbTransactionsFake.calledOnce).to.equal(true);
						expect(dbTransactionsFake.firstCall.args[1]).to.deep.equal(pagingBag);
						paginationParser.restore();
					});
				});

				it('allowed sort fields are taken into account', () => {
					// Arrange:
					const paginationParserSpy = sinon.spy(routeUtils, 'parsePaginationArguments');
					const expectedAllowedSortFields = ['_id'];

					// Act:
					return mockServer.callRoute(route, { params: {} }).then(() => {
						// Assert:
						expect(paginationParserSpy.calledOnce).to.equal(true);
						expect(paginationParserSpy.firstCall.args[2]).to.deep.equal(expectedAllowedSortFields);
						paginationParserSpy.restore();
					});
				});
			});

			describe('does not allow filtering by address if signerPublicKey or recipientAddress are provided', () => {
				const errorMessage = 'can\'t filter by address if signerPublicKey or recipientAddress are already provided';

				it('address and signer public key', () => {
					const req = {
						params: { address: testAddressString, signerPublicKey: testPublickeyString }
					};

					// Act + Assert
					expect(() => mockServer.callRoute(route, req)).to.throw(errorMessage);
				});

				it('address and recipient address', () => {
					const req = {
						params: { address: testAddressString, recipientAddress: testAddressString }
					};

					// Act + Assert
					expect(() => mockServer.callRoute(route, req)).to.throw(errorMessage);
				});
			});

			describe('checks correct group is provided', () => {
				const runValidGroupTest = group => {
					it(group, () =>
						// Act + Assert
						mockServer.callRoute(route, { params: { group } }).then(() => {
							expect(dbTransactionsFake.firstCall.args[0]).to.deep.equal({
								address: undefined,
								height: undefined,
								recipientAddress: undefined,
								signerPublicKey: undefined,
								group,
								embedded: undefined,
								transactionTypes: undefined
							});
						}));
				};

				['confirmed', 'unconfirmed', 'partial'].forEach(group => runValidGroupTest(group));

				it('invalid', () => {
					const req = {
						params: { group: 'nonsenseGroup' }
					};

					// Act + Assert
					expect(() => mockServer.callRoute(route, req)).to.throw('invalid transaction group provided');
				});
			});
		});
	});
});
