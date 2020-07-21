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

const TransactionGroups = {
	confirmed: 'confirmed',
	unconfirmed: 'unconfirmed',
	partial: 'partial'
};

describe('transaction routes', () => {
	describe('transactions', () => {
		const validObjectId = 'CCDDEEFF0011223344556677';
		const validHash = '112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00';

		describe('get', () => {
			describe('by id', () => {
				const fakeTransaction = { meta: { addresses: [] }, transaction: { type: 12345 } };

				const dbTransactionsByIdsFake = sinon.fake.resolves(fakeTransaction);
				const dbTransactionsByHashesFake = sinon.fake.resolves(fakeTransaction);

				const mockServer = new MockServer();
				const db = {
					transactionsByIds: dbTransactionsByIdsFake,
					transactionsByHashes: dbTransactionsByHashesFake
				};
				transactionRoutes.register(mockServer.server, db, {});

				const route = mockServer.getRoute('/transactions/:group/:transactionId').get();

				beforeEach(() => {
					mockServer.resetStats();
					dbTransactionsByIdsFake.resetHistory();
					dbTransactionsByHashesFake.resetHistory();
				});

				const runParseArgumentParamTest = (params, parserName) => {
					// Arrange
					const req = { params: { group: TransactionGroups.confirmed, transactionId: params } };
					const parseArgumentSpy = sinon.spy(routeUtils, 'parseArgument');

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(parseArgumentSpy.calledOnceWith(
							{ group: TransactionGroups.confirmed, transactionId: params },
							'transactionId',
							parserName
						)).to.equal(true);
						parseArgumentSpy.restore();
					});
				};

				it('throws if invalid length of transaction id', () => {
					// Arrange
					const req = { params: { group: TransactionGroups.confirmed, transactionId: '12345' } };

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('invalid length of transaction id \'12345\'');
				});

				it('calls parseArgument with correct parser for id', () => runParseArgumentParamTest(validObjectId, 'objectId'));
				it('calls parseArgument with correct parser for hash', () => runParseArgumentParamTest(validHash, 'hash256'));

				describe('checks correct group is provided', () => {
					const runValidGroupTest = group => {
						it(`${group} - by id`, () => {
							// Arrange
							const req = { params: { group, transactionId: validObjectId } };

							// Act:
							mockServer.callRoute(route, req).then(() => {
								// Assert:
								expect(dbTransactionsByIdsFake.calledOnce).to.equal(true);
								expect(dbTransactionsByIdsFake.firstCall.args[0]).to.equal(group);
							});
						});

						it(`${group} - by hash`, () => {
							// Arrange
							const req = { params: { group, transactionId: validHash } };

							// Act:
							mockServer.callRoute(route, req).then(() => {
								// Assert:
								expect(dbTransactionsByHashesFake.calledOnce).to.equal(true);
								expect(dbTransactionsByHashesFake.firstCall.args[0]).to.equal(group);
							});
						});
					};

					Object.keys(TransactionGroups).forEach(group => runValidGroupTest(group));

					it('not found if group does not exists', () => {
						// Arrange
						const req = { params: { group: 'nonExistingGroup', transactionId: validObjectId } };

						// Act:
						mockServer.callRoute(route, req);

						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
						expect(mockServer.next.firstCall.args[0].statusCode).to.equal(404);
					});
				});

				describe('calls correct transaction retriever with correct params', () => {
					it('id param', () => {
						// Arrange
						const req = { params: { group: TransactionGroups.confirmed, transactionId: validObjectId } };

						// Act:
						return mockServer.callRoute(route, req).then(() => {
							// Assert:
							expect(dbTransactionsByIdsFake.calledOnce).to.equal(true);
							expect(dbTransactionsByIdsFake.firstCall.args[1]).to.deep.equal([validObjectId]);

							expect(mockServer.send.firstCall.args[0]).to.deep.equal({
								payload: fakeTransaction,
								type: routeResultTypes.transaction
							});
							expect(mockServer.next.calledOnce).to.equal(true);
						});
					});

					it('hash param', () => {
						// Arrange
						const req = { params: { group: TransactionGroups.confirmed, transactionId: validHash } };

						// Act:
						return mockServer.callRoute(route, req).then(() => {
							// Assert:
							expect(dbTransactionsByHashesFake.calledOnce).to.equal(true);
							expect(dbTransactionsByHashesFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(validHash)]);

							expect(mockServer.send.firstCall.args[0]).to.deep.equal({
								payload: fakeTransaction,
								type: routeResultTypes.transaction
							});
							expect(mockServer.next.calledOnce).to.equal(true);
						});
					});
				});
			});

			describe('paginated', () => {
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

				const route = mockServer.getRoute('/transactions/:group').get();

				beforeEach(() => {
					mockServer.resetStats();
					dbTransactionsFake.resetHistory();
				});

				describe('returns transactions', () => {
					it('returns correct structure with transactions', () => {
						const req = {
							params: { group: TransactionGroups.confirmed }
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
							const req = { params: { group: TransactionGroups.confirmed, [filter]: param } };

							const expectedResult = {
								address: undefined,
								height: undefined,
								recipientAddress: undefined,
								signerPublicKey: undefined,
								embedded: undefined,
								transactionTypes: undefined
							};

							expectedResult[filter] = value;

							// Act + Assert
							return mockServer.callRoute(route, req).then(() => {
								expect(dbTransactionsFake.firstCall.args[1]).to.deep.equal(expectedResult);
							});
						});
					};

					const testCases = [
						{ filter: 'height', param: '15', value: [15, 0] },
						{ filter: 'address', param: testAddressString, value: testAddress },
						{ filter: 'signerPublicKey', param: testPublickeyString, value: testPublickey },
						{ filter: 'recipientAddress', param: testAddressString, value: testAddress },
						{ filter: 'embedded', param: 'true', value: true }
					];

					testCases.forEach(testCase => {
						runParseFilterTest(testCase.filter, testCase.param, testCase.value);
					});

					it('transactionTypes', () => {
						const req = { params: { group: TransactionGroups.confirmed, type: ['1', '5', '25'] } };

						// Act + Assert
						return mockServer.callRoute(route, req).then(() => {
							expect(dbTransactionsFake.firstCall.args[1]).to.deep.equal({
								address: undefined,
								height: undefined,
								recipientAddress: undefined,
								signerPublicKey: undefined,
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
						const req = { params: { group: TransactionGroups.confirmed } };
						return mockServer.callRoute(route, req).then(() => {
							// Assert:
							expect(paginationParser.firstCall.args[0]).to.deep.equal(req.params);
							expect(paginationParser.firstCall.args[2]).to.deep.equal({ id: 'objectId' });

							expect(dbTransactionsFake.calledOnce).to.equal(true);
							expect(dbTransactionsFake.firstCall.args[2]).to.deep.equal(pagingBag);
							paginationParser.restore();
						});
					});

					it('allowed sort fields are taken into account', () => {
						// Arrange:
						const paginationParserSpy = sinon.spy(routeUtils, 'parsePaginationArguments');
						const expectedAllowedSortFields = { id: 'objectId' };

						// Act:
						return mockServer.callRoute(route, { params: { group: TransactionGroups.confirmed } }).then(() => {
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
							params: { group: TransactionGroups.confirmed, address: testAddressString, signerPublicKey: testPublickeyString }
						};

						// Act + Assert
						expect(() => mockServer.callRoute(route, req)).to.throw(errorMessage);
					});

					it('address and recipient address', () => {
						const req = {
							params: { group: TransactionGroups.confirmed, address: testAddressString, recipientAddress: testAddressString }
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
								expect(dbTransactionsFake.firstCall.args[1]).to.deep.equal({
									address: undefined,
									height: undefined,
									recipientAddress: undefined,
									signerPublicKey: undefined,
									embedded: undefined,
									transactionTypes: undefined
								});
							}));
					};

					Object.keys(TransactionGroups).forEach(group => runValidGroupTest(group));

					it('not found if group does not exists', () => {
						// Arrange:
						const req = { params: { group: 'nonExistingGroup' } };

						// Act:
						mockServer.callRoute(route, req);

						// Assert:
						expect(mockServer.next.calledOnce).to.equal(true);
						expect(mockServer.next.firstCall.args[0].statusCode).to.equal(404);
					});
				});
			});
		});

		describe('post', () => {
			const fakeTransactions = [{ meta: { addresses: [] }, transaction: { type: 12345 } }];

			const dbTransactionsByIdsFake = sinon.fake.resolves(fakeTransactions);
			const dbTransactionsByHashesFake = sinon.fake.resolves(fakeTransactions);

			const mockServer = new MockServer();
			const db = {
				transactionsByIds: dbTransactionsByIdsFake,
				transactionsByHashes: dbTransactionsByHashesFake
			};
			transactionRoutes.register(mockServer.server, db, {});

			const route = mockServer.getRoute('/transactions/:group').post();

			beforeEach(() => {
				mockServer.resetStats();
				dbTransactionsByIdsFake.resetHistory();
				dbTransactionsByHashesFake.resetHistory();
			});

			it('throws if ids or hashes are not provided', () => {
				// Arrange
				const req = { params: { group: TransactionGroups.confirmed } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('either ids or hashes must be provided');
			});

			it('throws if both ids and hashes are provided', () => {
				// Arrange
				const req = { params: { group: TransactionGroups.confirmed, transactionIds: [], hashes: [] } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('either ids or hashes must be provided');
			});

			describe('checks correct group is provided', () => {
				const runValidGroupTest = group => {
					it(`${group} - by ids`, () => {
						// Arrange
						const req = { params: { group, transactionIds: [validObjectId] } };

						// Act:
						mockServer.callRoute(route, req).then(() => {
							// Assert:
							expect(dbTransactionsByIdsFake.calledOnce).to.equal(true);
							expect(dbTransactionsByIdsFake.firstCall.args[0]).to.equal(group);
						});
					});
					it(`${group} - by hashes`, () => {
						// Arrange
						const req = { params: { group, hashes: [validHash] } };

						// Act:
						mockServer.callRoute(route, req).then(() => {
							// Assert:
							expect(dbTransactionsByHashesFake.calledOnce).to.equal(true);
							expect(dbTransactionsByHashesFake.firstCall.args[0]).to.equal(group);
						});
					});
				};

				Object.keys(TransactionGroups).forEach(group => runValidGroupTest(group));

				it('not found if group does not exists', () => {
					// Arrange
					const req = { params: { group: 'nonExistingGroup', transactionIds: [validObjectId] } };

					// Act:
					mockServer.callRoute(route, req);

					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.next.firstCall.args[0].statusCode).to.equal(404);
				});
			});

			const runParseArgumentAsArrayParamTest = (paramValues, paramName, parserName) => {
				// Arrange
				const req = { params: { group: TransactionGroups.confirmed, [paramName]: paramValues } };
				const parseArgumentsAsArraySpy = sinon.spy(routeUtils, 'parseArgumentAsArray');

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(parseArgumentsAsArraySpy.calledOnceWith(
						{ group: TransactionGroups.confirmed, [paramName]: paramValues },
						paramName,
						parserName
					)).to.equal(true);
					parseArgumentsAsArraySpy.restore();
				});
			};

			it('calls parseArgumentAsArray with correct parser for ids', () =>
				runParseArgumentAsArrayParamTest([validObjectId, validObjectId], 'transactionIds', 'objectId'));
			it('calls parseArgumentAsArray with correct parser for hashes', () =>
				runParseArgumentAsArrayParamTest([validHash, validHash], 'hashes', 'hash256'));

			describe('calls correct transaction retriever with correct params', () => {
				it('id params', () => {
					// Arrange
					const req = { params: { group: TransactionGroups.confirmed, transactionIds: [validObjectId] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbTransactionsByIdsFake.calledOnce).to.equal(true);
						expect(dbTransactionsByIdsFake.firstCall.args[1]).to.deep.equal([validObjectId]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakeTransactions,
							type: routeResultTypes.transaction
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('hash params', () => {
					// Arrange
					const req = { params: { group: TransactionGroups.confirmed, hashes: [validHash] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbTransactionsByHashesFake.calledOnce).to.equal(true);
						expect(dbTransactionsByHashesFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(validHash)]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakeTransactions,
							type: routeResultTypes.transaction
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('hash params (provisional test until params are split into transactionIds/hashes)', () => {
					// Arrange
					const req = { params: { group: TransactionGroups.confirmed, transactionIds: [validHash] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbTransactionsByHashesFake.calledOnce).to.equal(true);
						expect(dbTransactionsByHashesFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(validHash)]);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakeTransactions,
							type: routeResultTypes.transaction
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			});
		});

		describe('put', () => {
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
	});
});
