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
const MongoDb = require('mongodb');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { Binary } = MongoDb;
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
		const testPublicKey = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';
		const uint8TestPublicKey = convert.hexToUint8(testPublicKey);

		const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV';

		const nonExistingTestAddress = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA';
		const uint8NonExistingTestAddress = address.stringToAddress(nonExistingTestAddress);

		const getTestsBy = accountIdType => {
			const fakeTransactions = [{ meta: { addresses: [] }, transaction: { type: 12345 } }];
			const dbTransactionsFake = sinon.fake.resolves(fakeTransactions);
			const mockServer = new MockServer();

			const db = {
				accountTransactionsConfirmed: dbTransactionsFake,
				accountTransactionsUnconfirmed: dbTransactionsFake,
				accountTransactionsPartial: dbTransactionsFake,
				accountTransactionsIncoming: dbTransactionsFake,
				accountTransactionsOutgoing: dbTransactionsFake,
				addressToPublicKey: searchedAddress => {
					if (Buffer.from(searchedAddress).equals(Buffer.from(uint8NonExistingTestAddress)))
						return Promise.reject(Error('account not found'));

					return Promise.resolve({ account: { publicKey: new Binary(Buffer.from(uint8TestPublicKey)) } });
				}
			};
			const services = {
				config: {
					network: { name: 'mijinTest' },
					transactionStates: [{ dbPostfix: 'Partial', routePostfix: '/partial' }]
				}
			};

			accountRoutes.register(mockServer.server, db, services);

			beforeEach(() => {
				mockServer.resetStats();
				dbTransactionsFake.resetHistory();
			});

			const basicQueryTest = route =>
				it('basic query', () => {
					// Arrange:
					const req = { params: { accountId: AccountType.publicKey === accountIdType ? testPublicKey : testAddress } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbTransactionsFake.calledOnce).to.equal(true);
						expect(dbTransactionsFake.firstCall.args[0]).to.deep.equal(address.stringToAddress(testAddress));

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakeTransactions,
							type: 'transactionWithMetadata'
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

			const paginationParamsTest = route =>
				it('parses and fordwards pagination params correctly', () => {
					// Arrange:
					const req = {
						params: {
							accountId: testPublicKey,
							id: '00123456789AABBBCCDDEEFF',
							pageSize: '25',
							ordering: 'id'
						}
					};

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbTransactionsFake.calledOnce).to.equal(true);
						expect(dbTransactionsFake.firstCall.args[1]).to.equal('00123456789AABBBCCDDEEFF');
						expect(dbTransactionsFake.firstCall.args[2]).to.equal(25);
						expect(dbTransactionsFake.firstCall.args[3]).to.equal(1);
					});
				});

			const invalidPageIdTest = route =>
				it('returns 409 if invalid pageId', () => {
					// Arrange:
					const req = { params: { accountId: testPublicKey, id: '12345' } };

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('id is not a valid object id');
				});

			const invalidPageSizeTest = route =>
				it('returns 409 if invalid pageSize', () => {
					// Arrange:
					const req = { params: { accountId: testPublicKey, id: '00123456789AABBBCCDDEEFF', pageSize: '-1' } };

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('pageSize is not a valid unsigned integer');
				});

			const invalidAccountTest = route =>
				it('returns 409 if accountId is invalid', () => {
					// Arrange:
					const req = { params: { accountId: 'aabbccddeeff' } };

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('accountId has an invalid format');
				});

			const getStandardTests = route => {
				basicQueryTest(route);
				paginationParamsTest(route);
				invalidPageIdTest(route);
				invalidPageSizeTest(route);
				invalidAccountTest(route);
			};

			describe('confirmed', () => getStandardTests(mockServer.getRoute('/account/:accountId/transactions').get()));

			describe('incoming', () => getStandardTests(mockServer.getRoute('/account/:accountId/transactions/incoming').get()));

			describe('unconfirmed', () => getStandardTests(mockServer.getRoute('/account/:accountId/transactions/unconfirmed').get()));

			describe('partial', () => getStandardTests(mockServer.getRoute('/account/:accountId/transactions/partial').get()));

			describe('outgoing', () => {
				const route = mockServer.getRoute('/account/:accountId/transactions/outgoing').get();

				it('basic query', () => {
					// Arrange:
					const req = { params: { accountId: AccountType.publicKey === accountIdType ? testPublicKey : testAddress } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbTransactionsFake.calledOnce).to.equal(true);
						expect(dbTransactionsFake.firstCall.args[0]).to.deep.equal(uint8TestPublicKey);

						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakeTransactions,
							type: 'transactionWithMetadata'
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});

				// tests only nedded when searching by address
				if (AccountType.address === accountIdType) {
					it('account does not exists or does not have public key', () => {
						// Arrange:
						const req = { params: { accountId: nonExistingTestAddress } };

						// Act:
						return mockServer.callRoute(route, req).then(() => {
							// Assert:
							expect(dbTransactionsFake.calledOnce).to.equal(false);

							expect(mockServer.send.firstCall.args[0]).to.deep.equal({
								payload: [],
								type: 'transactionWithMetadata'
							});
							expect(mockServer.next.calledOnce).to.equal(true);
						});
					});
				}

				paginationParamsTest(route);
				invalidPageIdTest(route);
				invalidPageSizeTest(route);
				invalidAccountTest(route);
			});
		};

		describe('by publicKey', () => getTestsBy(AccountType.publicKey));
		describe('by address', () => getTestsBy(AccountType.address));
	});
});
