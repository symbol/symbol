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

const AccountType = require('../../../src/plugins/AccountType');
const mosaicRoutes = require('../../../src/plugins/mosaic/mosaicRoutes');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address } = catapult.model;
const { convert } = catapult.utils;

describe('mosaic routes', () => {
	describe('by id', () => {
		const mosaicIds = ['1234567890ABCDEF', 'ABCDEF0123456789'];
		const uint64MosaicIds = [[0x90ABCDEF, 0x12345678], [0x23456789, 0xABCDEF01]];
		const errorMessage = 'has an invalid format';
		test.route.document.addGetPostDocumentRouteTests(mosaicRoutes.register, {
			routes: { singular: '/mosaic/:mosaicId', plural: '/mosaic' },
			inputs: {
				valid: { object: { mosaicId: mosaicIds[0] }, parsed: [uint64MosaicIds[0]], printable: mosaicIds[0] },
				validMultiple: { object: { mosaicIds }, parsed: uint64MosaicIds },
				invalid: { object: { mosaicId: '12345' }, error: `mosaicId ${errorMessage}` },
				invalidMultiple: {
					object: { mosaicIds: [mosaicIds[0], '12345', mosaicIds[1]] },
					error: `element in array mosaicIds ${errorMessage}`
				}
			},
			dbApiName: 'mosaicsByIds',
			type: 'mosaicDescriptor'
		});
	});

	describe('by owner', () => {
		const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV';
		const testPublicKey = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';
		const nonExistingPublicKey = '75D8BB873DA8F5CCA741435DE76A46AFC2840803EBF080E931195B048D77F88C';
		const nonExistingTestAddress = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG';

		const ownedMosaicsSample = {
			id: '',
			supply: 1,
			startHeight: '',
			ownerPublicKey: '',
			ownerAddress: '',
			revision: 1,
			flags: 3,
			divisibility: 3,
			duration: ''
		};

		const dbMosaicsByOwnersFake = sinon.fake((type, accountIds) => {
			if (Buffer.from(accountIds[0]).equals(Buffer.from(address.stringToAddress(nonExistingTestAddress)))
				|| Buffer.from(accountIds[0]).equals(Buffer.from(convert.hexToUint8(nonExistingPublicKey))))
				return Promise.resolve([]);

			return Promise.resolve([ownedMosaicsSample]);
		});

		const mockServer = new MockServer();
		const db = { mosaicsByOwners: dbMosaicsByOwnersFake };
		mosaicRoutes.register(mockServer.server, db);

		beforeEach(() => {
			mockServer.resetStats();
			dbMosaicsByOwnersFake.resetHistory();
		});

		describe('GET', () => {
			it('returns empty if address not found', () => {
				// Arrange:
				const req = { params: { accountId: nonExistingTestAddress } };
				const route = mockServer.getRoute('/account/:accountId/mosaics').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsByOwnersFake.calledOnce).to.equal(true);
					expect(dbMosaicsByOwnersFake.firstCall.args[0]).to.deep.equal(AccountType.address);
					expect(dbMosaicsByOwnersFake.firstCall.args[1]).to.deep.equal([address.stringToAddress(nonExistingTestAddress)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [],
						type: 'mosaicDescriptor.mosaic'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns empty if publicKey not found', () => {
				// Arrange:
				const req = { params: { accountId: nonExistingPublicKey } };
				const route = mockServer.getRoute('/account/:accountId/mosaics').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsByOwnersFake.calledOnce).to.equal(true);
					expect(dbMosaicsByOwnersFake.firstCall.args[0]).to.deep.equal(AccountType.publicKey);
					expect(dbMosaicsByOwnersFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(nonExistingPublicKey)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [],
						type: 'mosaicDescriptor.mosaic'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns owned mosaics by address', () => {
				// Arrange:
				const req = { params: { accountId: testAddress } };
				const route = mockServer.getRoute('/account/:accountId/mosaics').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsByOwnersFake.calledOnce).to.equal(true);
					expect(dbMosaicsByOwnersFake.firstCall.args[0]).to.deep.equal(AccountType.address);
					expect(dbMosaicsByOwnersFake.firstCall.args[1]).to.deep.equal([address.stringToAddress(testAddress)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [ownedMosaicsSample],
						type: 'mosaicDescriptor.mosaic'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns owned mosaics by publicKey', () => {
				// Arrange:
				const req = { params: { accountId: testPublicKey } };
				const route = mockServer.getRoute('/account/:accountId/mosaics').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsByOwnersFake.calledOnce).to.equal(true);
					expect(dbMosaicsByOwnersFake.firstCall.args[0]).to.deep.equal(AccountType.publicKey);
					expect(dbMosaicsByOwnersFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(testPublicKey)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [ownedMosaicsSample],
						type: 'mosaicDescriptor.mosaic'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('throws error if accountId is invalid', () => {
				// Arrange:
				const req = { params: { accountId: 'AB12345' } };
				const route = mockServer.getRoute('/account/:accountId/mosaics').get();

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('accountId has an invalid format');
			});
		});

		describe('POST', () => {
			it('returns empty if address not found', () => {
				// Arrange:
				const req = { params: { addresses: [nonExistingTestAddress] } };
				const route = mockServer.getRoute('/account/mosaics').post();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsByOwnersFake.calledOnce).to.equal(true);
					expect(dbMosaicsByOwnersFake.firstCall.args[0]).to.deep.equal(AccountType.address);
					expect(dbMosaicsByOwnersFake.firstCall.args[1]).to.deep.equal([address.stringToAddress(nonExistingTestAddress)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [],
						type: 'mosaicDescriptor.mosaic'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns empty if publicKey not found', () => {
				// Arrange:
				const req = { params: { publicKeys: [nonExistingPublicKey] } };
				const route = mockServer.getRoute('/account/mosaics').post();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsByOwnersFake.calledOnce).to.equal(true);
					expect(dbMosaicsByOwnersFake.firstCall.args[0]).to.deep.equal(AccountType.publicKey);
					expect(dbMosaicsByOwnersFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(nonExistingPublicKey)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [],
						type: 'mosaicDescriptor.mosaic'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns owned mosaics by address', () => {
				// Arrange:
				const req = { params: { addresses: [testAddress] } };
				const route = mockServer.getRoute('/account/mosaics').post();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsByOwnersFake.calledOnce).to.equal(true);
					expect(dbMosaicsByOwnersFake.firstCall.args[0]).to.deep.equal(AccountType.address);
					expect(dbMosaicsByOwnersFake.firstCall.args[1]).to.deep.equal([address.stringToAddress(testAddress)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [ownedMosaicsSample],
						type: 'mosaicDescriptor.mosaic'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('returns owned mosaics by publicKey', () => {
				// Arrange:
				const req = { params: { publicKeys: [testPublicKey] } };
				const route = mockServer.getRoute('/account/mosaics').post();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicsByOwnersFake.calledOnce).to.equal(true);
					expect(dbMosaicsByOwnersFake.firstCall.args[0]).to.deep.equal(AccountType.publicKey);
					expect(dbMosaicsByOwnersFake.firstCall.args[1]).to.deep.equal([convert.hexToUint8(testPublicKey)]);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [ownedMosaicsSample],
						type: 'mosaicDescriptor.mosaic'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('does not support publicKeys and addresses provided at the same time', () => {
				// Arrange:
				const req = { params: { addresses: [''], publicKeys: [''] } };
				const route = mockServer.getRoute('/account/mosaics').post();

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('publicKeys and addresses cannot both be provided');
			});

			it('throws error if addresses contains an invalid address', () => {
				// Arrange:
				const req = { params: { addresses: [testAddress, 'AAAAAAAAAA'] } };
				const route = mockServer.getRoute('/account/mosaics').post();

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('element in array addresses has an invalid format');
			});

			it('throws error if publicKeys contains an invalid publicKey', () => {
				// Arrange:
				const req = { params: { publicKeys: [testPublicKey, 'AAAAAAAAAA'] } };
				const route = mockServer.getRoute('/account/mosaics').post();

				// Act + Assert:
				expect(() => mockServer.callRoute(route, req)).to.throw('element in array publicKeys has an invalid format');
			});
		});
	});
});
