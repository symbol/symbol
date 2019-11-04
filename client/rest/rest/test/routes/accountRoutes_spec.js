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

const { test } = require('./utils/routeTestUtils');
const accountRoutes = require('../../src/routes/accountRoutes');
const { MockServer } = require('../../test/routes/utils/routeTestUtils');
const AccountType = require('../../src/plugins/AccountType');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { convert } = catapult.utils;
const { addresses, publicKeys } = test.sets;

describe('account routes', () => {
	describe('get by account', () => {
		const addGetTests = (key, ids, parsedIds, validBody, invalidBody, errorMessage) => {
			test.route.document.addGetPostDocumentRouteTests(accountRoutes.register, {
				routes: { singular: '/account/:accountId', plural: '/account' },
				inputs: {
					valid: { object: { accountId: ids[0] }, parsed: [{ [key]: parsedIds[0] }], printable: ids[0] },
					validMultiple: { object: validBody, parsed: parsedIds.map(parsedId => ({ [key]: parsedId })) },
					invalid: { object: { accountId: '12345' }, error: 'accountId has an invalid format' },
					invalidMultiple: {
						object: invalidBody,
						error: errorMessage
					}
				},
				dbApiName: 'accountsByIds',
				type: 'accountWithMetadata',
				config: { transactionStates: [] }
			});
		};

		describe('by address', () =>
			addGetTests(
				'address',
				addresses.valid,
				addresses.valid.map(address.stringToAddress),
				{ addresses: addresses.valid },
				{ addresses: [addresses.valid[0], '12345', addresses.valid[1]] },
				'element in array addresses has an invalid format'
			));

		describe('by publicKey', () =>
			addGetTests(
				'publicKey',
				publicKeys.valid,
				publicKeys.valid.map(convert.hexToUint8),
				{ publicKeys: publicKeys.valid },
				{ publicKeys: [publicKeys.valid[0], '12345', publicKeys.valid[1]] },
				'element in array publicKeys has an invalid format'
			));

		it('does not support publicKeys and addresses provided at the same time', () => {
			// Arrange:
			const keyGroups = [];
			const db = test.setup.createCapturingDb('accountsByIds', keyGroups, [{ value: 'this is nonsense' }]);

			// Act:
			const registerRoutes = accountRoutes.register;
			const errorMessage = 'publicKeys and addresses cannot both be provided';
			return test.route.executeThrows(
				registerRoutes,
				'/account',
				'post',
				{ addresses: addresses.valid, publicKeys: publicKeys.valid },
				db,
				{ transactionStates: [] },
				errorMessage,
				409
			);
		});
	});

	describe('account transactions', () => {
		describe('all, outgoing, unconfirmed, partial', () => {
			const addGetTestsBy = accountIdType => {
				const addAccountTransactionsTests = (apiPath, dbApiPath) => {
					describe(dbApiPath, () => {
						const pagingTestsFactory = test.setup.createPagingTestsFactory(
							{
								routes: accountRoutes,
								routeName: `/account/:accountId/${apiPath}`,
								createDb: (queriedIdentifiers, transactions) => ({
									[dbApiPath]: (publicKey, pageId, pageSize) => {
										queriedIdentifiers.push({ publicKey, pageId, pageSize });
										return Promise.resolve(transactions);
									},
									addressToPublicKey: () => Promise.resolve({
										account: { publicKey: { buffer: convert.hexToUint8(publicKeys.valid[0]) } }
									})
								}),
								config: { transactionStates: [{ dbPostfix: 'Partial', routePostfix: '/partial' }] }
							},
							{ accountId: accountIdType === AccountType.publicKey ? publicKeys.valid[0] : addresses.valid[0] },
							{ publicKey: convert.hexToUint8(publicKeys.valid[0]) },
							'transactionWithMetadata'
						);

						pagingTestsFactory.addDefault();
						pagingTestsFactory.addNonPagingParamFailureTest('accountId', '12345');
					});
				};

				// default transaction states
				addAccountTransactionsTests('transactions', 'accountTransactionsAll');
				addAccountTransactionsTests('transactions/outgoing', 'accountTransactionsOutgoing');
				addAccountTransactionsTests('transactions/unconfirmed', 'accountTransactionsUnconfirmed');

				// custom transaction states (enabled via custom configuration)
				addAccountTransactionsTests('transactions/partial', 'accountTransactionsPartial');

				const addOrderingParamTests = (apiPath, dbApiPath) => {
					describe(dbApiPath, () => {
						// Arrange:
						const createDb = (queriedIdentifiers, transactions) => ({
							[dbApiPath]: (publicKey, pageId, pageSize, ordering) => {
								queriedIdentifiers.push({
									publicKey, pageId, pageSize, ordering
								});
								return Promise.resolve(transactions);
							}
						});
						const keyGroups = [];
						const db = createDb(keyGroups, []);

						// Act:
						it('queries the database with ordering param', () => test.route.executeSingle(
							accountRoutes.register,
							`/account/:accountId/${apiPath}`,
							'get',
							Object.assign({}, { accountId: publicKeys.valid[0] }, { ordering: 'id' }),
							db,
							{ transactionStates: [{ dbPostfix: 'Partial', routePostfix: '/partial' }] },
							() => {
								// Assert:
								expect(keyGroups).to.deep.equal([Object.assign(
									{},
									{ publicKey: convert.hexToUint8(publicKeys.valid[0]) },
									{ pageId: undefined, pageSize: 0, ordering: 1 }
								)]);
							}
						));
					});
				};

				// default transaction states
				addOrderingParamTests('transactions', 'accountTransactionsAll');
				addOrderingParamTests('transactions/outgoing', 'accountTransactionsOutgoing');
				addOrderingParamTests('transactions/unconfirmed', 'accountTransactionsUnconfirmed');

				// custom transaction states (enabled via custom configuration)
				addOrderingParamTests('transactions/partial', 'accountTransactionsPartial');
			};

			describe('by publicKey', () => addGetTestsBy(AccountType.publicKey));

			describe('by address', () => addGetTestsBy(AccountType.address));
		});

		describe('incoming', () => {
			const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV';
			const testPublicKey = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';
			const testObjectId = '112233445566778899AABBCC';

			const transactionSample = { meta: { height: 1, index: 0 }, transaction: { id: 12345 } };
			const sentPayload = { payload: [transactionSample], type: 'transactionWithMetadata' };
			const dbIncomingFake = sinon.fake(() => Promise.resolve([transactionSample]));

			const mockServer = new MockServer();
			const db = { accountTransactionsIncoming: dbIncomingFake };
			const services = { config: { transactionStates: [], network: { name: 'mijinTest' } } };
			accountRoutes.register(mockServer.server, db, services);
			const route = mockServer.getRoute('/account/:accountId/transactions/incoming').get();

			beforeEach(() => {
				mockServer.resetStats();
				dbIncomingFake.resetHistory();
			});

			const testBy = accountId => {
				it('basic query', () => {
					// Arrange:
					const req = { params: { accountId } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbIncomingFake.calledOnce).to.equal(true);
						expect(dbIncomingFake.firstCall.args[0]).to.deep.equal(address.stringToAddress(testAddress));
						expect(mockServer.send.firstCall.args[0]).to.deep.equal(sentPayload);
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('query with pageId, pageSize and ordering', () => {
					// Arrange:
					const req = {
						params: {
							accountId,
							id: testObjectId,
							pageSize: '50',
							ordering: 'id'
						}
					};

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbIncomingFake.calledOnce).to.equal(true);
						expect(dbIncomingFake.firstCall.args[0]).to.deep.equal(address.stringToAddress(testAddress));
						expect(dbIncomingFake.firstCall.args[1]).to.deep.equal(testObjectId);
						expect(dbIncomingFake.firstCall.args[2]).to.deep.equal(50);
						expect(dbIncomingFake.firstCall.args[3]).to.deep.equal(1);
						expect(mockServer.send.firstCall.args[0]).to.deep.equal(sentPayload);
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				it('throws error if invalid pageId', () => {
					// Arrange:
					const req = {
						params: {
							accountId,
							id: 'alice',
							pageSize: '50',
							ordering: 'id'
						}
					};

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('id is not a valid object id');
				});

				it('throws error if invalid pageSize', () => {
					// Arrange:
					const req = {
						params: {
							accountId,
							id: testObjectId,
							pageSize: 'alice',
							ordering: 'id'
						}
					};

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('pageSize is not a valid unsigned integer');
				});

				it('throws error if accountId is invalid', () => {
					// Arrange:
					const req = { params: { accountId: 'ABCD' } };

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('accountId has an invalid format');
				});
			};

			describe('by publicKey', () => testBy(testPublicKey));

			describe('by address', () => testBy(testAddress));
		});
	});
});
