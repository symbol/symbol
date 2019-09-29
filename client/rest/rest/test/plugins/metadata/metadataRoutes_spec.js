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

const metadataRoutes = require('../../../src/plugins/metadata/metadataRoutes');
const routeUtils = require('../../../src/routes/routeUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const MongoDb = require('mongodb');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { Binary, Long } = MongoDb;
const { convert } = catapult.utils;
const { metadata } = catapult.model;

describe('metadata routes', () => {
	const testPublicKey = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';
	const uint8TestPublicKey = convert.hexToUint8(testPublicKey);

	const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV';
	const uint8TestAddress = address.stringToAddress(testAddress);

	const nonExistingTestAddress = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA';
	const uint8NonExistingTestAddress = address.stringToAddress(nonExistingTestAddress);

	const scopedMetadataKey = '0DC67FBE1CAD29E3';
	const uInt64ScopedMetadataKey = [0x1CAD29E3, 0x0DC67FBE];
	const senderPublicKey = '4A8876A9042FEB16670A119EB7264FE9C90A56DB64F90D4FEAF1320D0852F754';
	const uint8SenderPublicKey = convert.hexToUint8(senderPublicKey);

	const metadataEntry = {
		id: {},
		metadataEntry: {
			compositeHash: '',
			senderPublicKey: '',
			targetPublicKey: '',
			scopedMetadataKey: '',
			targetId: '',
			metadataType: '',
			valueSize: '',
			value: ''
		}
	};

	const expectedArrayMetadataOutput = { payload: { metadataEntries: [metadataEntry] }, type: 'metadata' };
	const expectedSingleMetadataOutput = { payload: metadataEntry, type: 'metadata.entry' };

	const dbGetMetadataWithPaginationFake = sinon.fake.resolves([metadataEntry]);
	const dbGetMetadataByKeyFake = sinon.fake.resolves([metadataEntry]);
	const dbGetMetadataByKeyAndSenderFake = sinon.fake.resolves(metadataEntry);

	const mockServer = new MockServer();
	const db = {
		catapultDb: {
			addressToPublicKey: searchedAddress => {
				if (Buffer.from(searchedAddress).equals(Buffer.from(uint8NonExistingTestAddress)))
					return Promise.reject(Error('account not found'));

				return Promise.resolve({ account: { publicKey: new Binary(Buffer.from(uint8TestPublicKey)) } });
			}
		},
		getMetadataWithPagination: dbGetMetadataWithPaginationFake,
		getMetadataByKey: dbGetMetadataByKeyFake,
		getMetadataByKeyAndSender: dbGetMetadataByKeyAndSenderFake
	};
	metadataRoutes.register(mockServer.server, db, {});

	beforeEach(() => {
		mockServer.resetStats();
		dbGetMetadataWithPaginationFake.resetHistory();
		dbGetMetadataByKeyFake.resetHistory();
		dbGetMetadataByKeyAndSenderFake.resetHistory();
	});

	describe('account metadata', () => {
		describe('get account publicKey', () => {
			const addressToPublicKeySpy = sinon.spy(routeUtils, 'addressToPublicKey');

			beforeEach(() => addressToPublicKeySpy.resetHistory());

			after(() => addressToPublicKeySpy.restore());

			it('can get publicKey from address, getting metadata', () => {
				// Arrange:
				const accountFilter = { 'metadataEntry.targetPublicKey': uint8TestPublicKey };
				const req = { params: { accountId: testAddress } };
				const route = mockServer.getRoute('/account/:accountId/metadata').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(addressToPublicKeySpy.calledOnce).to.equal(true);
					expect(addressToPublicKeySpy.firstCall.args[1]).to.deep.equal(uint8TestAddress);

					expect(dbGetMetadataWithPaginationFake.calledOnce).to.equal(true);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[1]).to.deep.equal(accountFilter);
				});
			});

			it('returns resource not found if address doesn\'t exist, getting metadata', () => {
				// Arrange:
				const req = { params: { accountId: nonExistingTestAddress } };
				const route = mockServer.getRoute('/account/:accountId/metadata').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.send.firstCall.args[0].body).to.deep.equal(
						{ code: 'ResourceNotFound', message: `no resource exists with id '${nonExistingTestAddress}'` }
					);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('can get publicKey from address, getting metadata by key', () => {
				// Arrange:
				const accountFilter = { 'metadataEntry.targetPublicKey': uint8TestPublicKey };
				const req = { params: { accountId: testAddress, key: scopedMetadataKey } };
				const route = mockServer.getRoute('/account/:accountId/metadata/:key').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(addressToPublicKeySpy.calledOnce).to.equal(true);
					expect(addressToPublicKeySpy.firstCall.args[1]).to.deep.equal(uint8TestAddress);

					expect(dbGetMetadataByKeyFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyFake.firstCall.args[1]).to.deep.equal(accountFilter);
				});
			});

			it('returns resource not found if address doesn\'t exist, getting metadata by key', () => {
				// Arrange:
				const req = { params: { accountId: nonExistingTestAddress, key: scopedMetadataKey } };
				const route = mockServer.getRoute('/account/:accountId/metadata/:key').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.send.firstCall.args[0].body).to.deep.equal(
						{ code: 'ResourceNotFound', message: `no resource exists with id '${nonExistingTestAddress}'` }
					);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('can get publicKey from address, getting metadata by key and sender', () => {
				// Arrange:
				const accountFilter = { 'metadataEntry.targetPublicKey': uint8TestPublicKey };
				const req = { params: { accountId: testAddress, key: scopedMetadataKey, publicKey: senderPublicKey } };
				const route = mockServer.getRoute('/account/:accountId/metadata/:key/sender/:publicKey').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(addressToPublicKeySpy.calledOnce).to.equal(true);
					expect(addressToPublicKeySpy.firstCall.args[1]).to.deep.equal(uint8TestAddress);

					expect(dbGetMetadataByKeyAndSenderFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[1]).to.deep.equal(accountFilter);
				});
			});

			it('returns resource not found if address doesn\'t exist, getting metadata by key and sender', () => {
				// Arrange:
				const req = { params: { accountId: nonExistingTestAddress, key: scopedMetadataKey, publicKey: senderPublicKey } };
				const route = mockServer.getRoute('/account/:accountId/metadata/:key/sender/:publicKey').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.send.firstCall.args[0].body).to.deep.equal(
						{ code: 'ResourceNotFound', message: `no resource exists with id '${nonExistingTestAddress}'` }
					);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});

		describe('get metadata', () => {
			it('can get metadata', () => {
				// Arrange:
				const accountFilter = { 'metadataEntry.targetPublicKey': uint8TestPublicKey };
				const req = { params: { accountId: testPublicKey } };
				const route = mockServer.getRoute('/account/:accountId/metadata').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataWithPaginationFake.calledOnce).to.equal(true);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[0]).to.equal(metadata.metadataType.account);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[1]).to.deep.equal(accountFilter);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('can get metadata with pagination', () => {
				// Arrange:
				const accountFilter = { 'metadataEntry.targetPublicKey': uint8TestPublicKey };
				const pagingId = '5b88a35e6e2a2b1801a857fa';
				const req = {
					params: {
						accountId: testPublicKey,
						id: pagingId,
						pageSize: '10',
						ordering: 'id'
					}
				};
				const route = mockServer.getRoute('/account/:accountId/metadata').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataWithPaginationFake.calledOnce).to.equal(true);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[0]).to.equal(metadata.metadataType.account);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[1]).to.deep.equal(accountFilter);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[2]).to.equal(pagingId);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[3]).to.equal(10);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[4]).to.equal(1);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});

		describe('get metadata by key', () => {
			it('can get metadata by key', () => {
				// Arrange:
				const accountFilter = { 'metadataEntry.targetPublicKey': uint8TestPublicKey };
				const req = { params: { accountId: testPublicKey, key: scopedMetadataKey } };
				const route = mockServer.getRoute('/account/:accountId/metadata/:key').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyFake.firstCall.args[0]).to.equal(metadata.metadataType.account);
					expect(dbGetMetadataByKeyFake.firstCall.args[1]).to.deep.equal(accountFilter);
					expect(dbGetMetadataByKeyFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});

		describe('get metadata by key and sender', () => {
			it('can get metadata by key and sender', () => {
				// Arrange:
				const accountFilter = { 'metadataEntry.targetPublicKey': uint8TestPublicKey };
				const req = { params: { accountId: testPublicKey, key: scopedMetadataKey, publicKey: senderPublicKey } };
				const route = mockServer.getRoute('/account/:accountId/metadata/:key/sender/:publicKey').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyAndSenderFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[0]).to.equal(metadata.metadataType.account);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[1]).to.deep.equal(accountFilter);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[3]).to.deep.equal(uint8SenderPublicKey);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedSingleMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});
	});

	describe('mosaic metadata', () => {
		const mosaicId = '30AC58CFA34E98AA';
		const longTypeMosaicId = new Long(0xA34E98AA, 0x30AC58CF);

		describe('get metadata', () => {
			it('can get metadata', () => {
				// Arrange:
				const mosaicFilter = { 'metadataEntry.targetId': longTypeMosaicId };
				const req = { params: { mosaicId } };
				const route = mockServer.getRoute('/mosaic/:mosaicId/metadata').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataWithPaginationFake.calledOnce).to.equal(true);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[0]).to.equal(metadata.metadataType.mosaic);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[1]).to.deep.equal(mosaicFilter);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('can get metadata with pagination', () => {
				// Arrange:
				const mosaicFilter = { 'metadataEntry.targetId': longTypeMosaicId };
				const pagingId = '5b88a35e6e2a2b1801a857fa';
				const req = {
					params: {
						mosaicId,
						id: pagingId,
						pageSize: '10',
						ordering: 'id'
					}
				};
				const route = mockServer.getRoute('/mosaic/:mosaicId/metadata').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataWithPaginationFake.calledOnce).to.equal(true);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[0]).to.equal(metadata.metadataType.mosaic);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[1]).to.deep.equal(mosaicFilter);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[2]).to.equal(pagingId);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[3]).to.equal(10);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[4]).to.equal(1);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});

		describe('get metadata by key', () => {
			it('can get metadata by key', () => {
				// Arrange:
				const mosaicFilter = { 'metadataEntry.targetId': longTypeMosaicId };
				const req = { params: { mosaicId, key: scopedMetadataKey } };
				const route = mockServer.getRoute('/mosaic/:mosaicId/metadata/:key').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyFake.firstCall.args[0]).to.equal(metadata.metadataType.mosaic);
					expect(dbGetMetadataByKeyFake.firstCall.args[1]).to.deep.equal(mosaicFilter);
					expect(dbGetMetadataByKeyFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});

		describe('get metadata by key and sender', () => {
			it('can get metadata by key and sender', () => {
				// Arrange:
				const mosaicFilter = { 'metadataEntry.targetId': longTypeMosaicId };
				const req = { params: { mosaicId, key: scopedMetadataKey, publicKey: senderPublicKey } };
				const route = mockServer.getRoute('/mosaic/:mosaicId/metadata/:key/sender/:publicKey').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyAndSenderFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[0]).to.equal(metadata.metadataType.mosaic);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[1]).to.deep.equal(mosaicFilter);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[3]).to.deep.equal(uint8SenderPublicKey);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedSingleMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});
	});

	describe('namespace metadata', () => {
		const namespaceId = '2833BA00C2565877';
		const longTypeNamespaceId = new Long(0xC2565877, 0x2833BA00);

		describe('get metadata', () => {
			it('can get metadata', () => {
				// Arrange:
				const namespaceFilterFilter = { 'metadataEntry.targetId': longTypeNamespaceId };
				const req = { params: { namespaceId } };
				const route = mockServer.getRoute('/namespace/:namespaceId/metadata').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataWithPaginationFake.calledOnce).to.equal(true);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[0]).to.equal(metadata.metadataType.namespace);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[1]).to.deep.equal(namespaceFilterFilter);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('can get metadata with pagination', () => {
				// Arrange:
				const namespaceFilterFilter = { 'metadataEntry.targetId': longTypeNamespaceId };
				const pagingId = '5b88a35e6e2a2b1801a857fa';
				const req = {
					params: {
						namespaceId,
						id: pagingId,
						pageSize: '10',
						ordering: 'id'
					}
				};
				const route = mockServer.getRoute('/namespace/:namespaceId/metadata').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataWithPaginationFake.calledOnce).to.equal(true);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[0]).to.equal(metadata.metadataType.namespace);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[1]).to.deep.equal(namespaceFilterFilter);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[2]).to.equal(pagingId);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[3]).to.equal(10);
					expect(dbGetMetadataWithPaginationFake.firstCall.args[4]).to.equal(1);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});

		describe('get metadata by key', () => {
			it('can get metadata by key', () => {
				// Arrange:
				const namespaceFilter = { 'metadataEntry.targetId': longTypeNamespaceId };
				const req = { params: { namespaceId, key: scopedMetadataKey } };
				const route = mockServer.getRoute('/namespace/:namespaceId/metadata/:key').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyFake.firstCall.args[0]).to.equal(metadata.metadataType.namespace);
					expect(dbGetMetadataByKeyFake.firstCall.args[1]).to.deep.equal(namespaceFilter);
					expect(dbGetMetadataByKeyFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedArrayMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});

		describe('get metadata by key and sender', () => {
			it('can get metadata by key and sender', () => {
				// Arrange:
				const namespaceFilter = { 'metadataEntry.targetId': longTypeNamespaceId };
				const req = { params: { namespaceId, key: scopedMetadataKey, publicKey: senderPublicKey } };
				const route = mockServer.getRoute('/namespace/:namespaceId/metadata/:key/sender/:publicKey').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyAndSenderFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[0]).to.equal(metadata.metadataType.namespace);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[1]).to.deep.equal(namespaceFilter);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[3]).to.deep.equal(uint8SenderPublicKey);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedSingleMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});
	});
});
