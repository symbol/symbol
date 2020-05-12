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
const { MockServer } = require('./utils/routeTestUtils');
const AccountType = require('../../src/plugins/AccountType');
const accountRoutes = require('../../src/routes/accountRoutes');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { address } = catapult.model;
const { Binary } = MongoDb;
const { convert } = catapult.utils;
const { addresses, publicKeys } = test.sets;

describe('account routes', () => {
	const testPublicKey = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';
	const uint8TestPublicKey = convert.hexToUint8(testPublicKey);
	const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV';
	const nonExistingTestAddress = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA';
	const uint8NonExistingTestAddress = address.stringToAddress(nonExistingTestAddress);

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

	const getAccountBlocksTest = (blockType, routePostFix, fieldName) => {
		describe(blockType, () => {
			const fakeBlocks = [
				{
					id: 0x5E3CD1498E18164DD5536133,
					meta: { hash: '' },
					block: { height: 1 }
				},
				{
					id: 0x4DD787654E887231D5111AAE,
					meta: { hash: '' },
					block: { height: 1 }
				}
			];

			const dbGetBlocksByFake = sinon.fake.resolves(fakeBlocks);
			const mockServer = new MockServer();

			const db = {
				getBlocksBy: dbGetBlocksByFake,
				addressToPublicKey: searchedAddress => {
					if (Buffer.from(searchedAddress).equals(Buffer.from(uint8NonExistingTestAddress)))
						return Promise.reject(Error('account not found'));

					return Promise.resolve({ account: { publicKey: new Binary(Buffer.from(uint8TestPublicKey)) } });
				}
			};

			const services = { config: { transactionStates: [] } };

			accountRoutes.register(mockServer.server, db, services);
			const route = mockServer.getRoute(`/account/:accountId${routePostFix}`).get();

			beforeEach(() => {
				mockServer.resetStats();
				dbGetBlocksByFake.resetHistory();
			});

			const getTestsBy = accountIdType => {
				it(`returns account blocks by ${accountIdType}`, () => {
					// Arrange:
					const req = { params: { accountId: AccountType.publicKey === accountIdType ? testPublicKey : testAddress } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbGetBlocksByFake.calledOnce).to.equal(true);
						expect(dbGetBlocksByFake.firstCall.args[0]).to.equal(fieldName);
						expect(dbGetBlocksByFake.firstCall.args[1]).to.deep.equal(uint8TestPublicKey);
						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: fakeBlocks,
							type: 'blockHeaderWithMetadataAndId'
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			};

			getTestsBy(AccountType.publicKey);
			getTestsBy(AccountType.address);

			it('returns empty if account can\'t be found by address', () => {
				// Arrange:
				const req = { params: { accountId: nonExistingTestAddress } };

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetBlocksByFake.calledOnce).to.equal(false);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [],
						type: 'blockHeaderWithMetadataAndId'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('parses and fordwards params correctly', () => {
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
					expect(dbGetBlocksByFake.calledOnce).to.equal(true);
					expect(dbGetBlocksByFake.firstCall.args[0]).to.equal(fieldName);
					expect(dbGetBlocksByFake.firstCall.args[1]).to.deep.equal(uint8TestPublicKey);
					expect(dbGetBlocksByFake.firstCall.args[2]).to.equal('00123456789AABBBCCDDEEFF');
					expect(dbGetBlocksByFake.firstCall.args[3]).to.equal(25);
					expect(dbGetBlocksByFake.firstCall.args[4]).to.equal(1);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: fakeBlocks,
						type: 'blockHeaderWithMetadataAndId'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns 409 if invalid pageId', () => {
				// Arrange:
				const req = { params: { accountId: testPublicKey, id: '12345' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('id is not a valid object id');
			});

			it('returns 409 if invalid pageSize', () => {
				// Arrange:
				const req = { params: { accountId: testPublicKey, id: '00123456789AABBBCCDDEEFF', pageSize: '-1' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('pageSize is not a valid unsigned integer');
			});

			it('returns 409 if accountId is invalid', () => {
				// Arrange:
				const req = { params: { accountId: 'aabbccddeeff' } };

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('accountId has an invalid format');
			});
		});
	};

	describe('account blocks', () => {
		getAccountBlocksTest('harvest', '/harvest', 'block.signerPublicKey');
		getAccountBlocksTest('beneficiary', '/beneficiary', 'block.beneficiaryPublicKey');
	});
});
