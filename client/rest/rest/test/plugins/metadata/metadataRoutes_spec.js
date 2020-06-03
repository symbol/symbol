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
const { MockServer } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const MongoDb = require('mongodb');
const sinon = require('sinon');

const { address } = catapult.model;
const { Long } = MongoDb;
const { metadata } = catapult.model;

describe('metadata routes', () => {
	const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ';
	const uint8TestAddress = address.stringToAddress(testAddress);

	const scopedMetadataKey = '0DC67FBE1CAD29E3';
	const uInt64ScopedMetadataKey = [0x1CAD29E3, 0x0DC67FBE];

	const senderAddress = 'SAMZMPX33DFIIVOCNJYMF5KJTGLAEVNKHHFROLQ';
	const uint8SenderAddress = address.stringToAddress(senderAddress);


	const metadataEntry = {
		id: {},
		metadataEntry: {
			compositeHash: '',
			sourceAddress: '',
			targetAddress: '',
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
		describe('get metadata', () => {
			it('can get metadata', () => {
				// Arrange:
				const accountFilter = { 'metadataEntry.targetAddress': uint8TestAddress };
				const req = { params: { address: testAddress } };
				const route = mockServer.getRoute('/metadata/account/:address').get();

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
				const accountFilter = { 'metadataEntry.targetAddress': uint8TestAddress };
				const pagingId = '5b88a35e6e2a2b1801a857fa';
				const req = {
					params: {
						address: testAddress,
						id: pagingId,
						pageSize: '10',
						ordering: 'id'
					}
				};
				const route = mockServer.getRoute('/metadata/account/:address').get();

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
				const accountFilter = { 'metadataEntry.targetAddress': uint8TestAddress };
				const req = { params: { address: testAddress, key: scopedMetadataKey } };
				const route = mockServer.getRoute('/metadata/account/:address/key/:key').get();

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
				const accountFilter = { 'metadataEntry.targetAddress': uint8TestAddress };
				const req = { params: { address: testAddress, key: scopedMetadataKey, senderAddress } };
				const route = mockServer.getRoute('/metadata/account/:address/key/:key/sender/:senderAddress').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyAndSenderFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[0]).to.equal(metadata.metadataType.account);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[1]).to.deep.equal(accountFilter);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[3]).to.deep.equal(uint8SenderAddress);
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
				const route = mockServer.getRoute('/metadata/mosaic/:mosaicId').get();

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
				const route = mockServer.getRoute('/metadata/mosaic/:mosaicId').get();

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
				const route = mockServer.getRoute('/metadata/mosaic/:mosaicId/key/:key').get();

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
				const req = { params: { mosaicId, key: scopedMetadataKey, senderAddress } };
				const route = mockServer.getRoute('/metadata/mosaic/:mosaicId/key/:key/sender/:senderAddress').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyAndSenderFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[0]).to.equal(metadata.metadataType.mosaic);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[1]).to.deep.equal(mosaicFilter);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[3]).to.deep.equal(uint8SenderAddress);
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
				const route = mockServer.getRoute('/metadata/namespace/:namespaceId').get();

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
				const route = mockServer.getRoute('/metadata/namespace/:namespaceId').get();

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
				const route = mockServer.getRoute('/metadata/namespace/:namespaceId/key/:key').get();

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
				const req = { params: { namespaceId, key: scopedMetadataKey, senderAddress } };
				const route = mockServer.getRoute('/metadata/namespace/:namespaceId/key/:key/sender/:senderAddress').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbGetMetadataByKeyAndSenderFake.calledOnce).to.equal(true);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[0]).to.equal(metadata.metadataType.namespace);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[1]).to.deep.equal(namespaceFilter);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[2]).to.deep.equal(uInt64ScopedMetadataKey);
					expect(dbGetMetadataByKeyAndSenderFake.firstCall.args[3]).to.deep.equal(uint8SenderAddress);
					expect(mockServer.send.firstCall.args[0]).to.deep.equal(expectedSingleMetadataOutput);
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});
	});
});
