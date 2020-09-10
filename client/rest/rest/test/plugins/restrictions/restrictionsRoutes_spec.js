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

const restrictionsRoutes = require('../../../src/plugins/restrictions/restrictionsRoutes');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { address, restriction } = catapult.model;
const { addresses } = test.sets;

describe('restrictions routes', () => {
	describe('account restrictions', () => {
		describe('get by address', () => {
			test.route.document.addGetDocumentRouteTests(restrictionsRoutes.register, {
				route: '/restrictions/account/:address',
				inputs: {
					valid: {
						object: { address: addresses.valid[0] },
						parsed: [[address.stringToAddress(addresses.valid[0])]],
						printable: addresses.valid[0]
					},
					invalid: {
						object: { address: addresses.invalid },
						error: 'address has an invalid format'
					}
				},
				dbApiName: 'accountRestrictionsByAddresses',
				type: 'accountRestrictions'
			});
		});
	});

	describe('mosaic restrictions', () => {
		const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXQ';
		const uint8TestAddress = address.stringToAddress(testAddress);

		const testMosaicIds = {
			one: {
				id: '0DC67FBE1CAD29E3',
				uInt64: [0x1CAD29E3, 0x0DC67FBE]
			},
			two: {
				id: '0DC67FBE1CAD29E3',
				uInt64: [0x1CAD29E3, 0x0DC67FBE]
			}
		};

		const mosaicGlobalRestrictionEntrySample = {
			mosaicRestrictionEntry: {
				compositeHash: '',
				entryType: 1,
				mosaicId: '',
				restrictions: [
					{
						key: '',
						restriction: {
							referenceMosaicId: 0,
							restrictionValue: 0,
							restrictionType: 5
						}
					}
				]
			}
		};
		const mosaicAddressRestrictionEntrySample = {
			mosaicRestrictionEntry: {
				compositeHash: '',
				entryType: 0,
				mosaicId: '',
				targetAddress: '',
				restrictions: [
					{
						key: 0,
						value: 0
					}
				]
			}
		};

		const dbMosaicRestrictionsByMosaicIdsFake = sinon.fake.resolves([mosaicGlobalRestrictionEntrySample]);
		const dbMosaicAddressRestrictionsFake = sinon.fake.resolves([mosaicAddressRestrictionEntrySample]);

		const mockServer = new MockServer();
		const db = {
			mosaicRestrictionsByMosaicIds: dbMosaicRestrictionsByMosaicIdsFake,
			mosaicAddressRestrictions: dbMosaicAddressRestrictionsFake
		};
		const services = { config: { network: { name: 'mijinTest' } } };
		restrictionsRoutes.register(mockServer.server, db, services);

		beforeEach(() => {
			mockServer.resetStats();
			dbMosaicRestrictionsByMosaicIdsFake.resetHistory();
			dbMosaicAddressRestrictionsFake.resetHistory();
		});

		describe('mosaic global restrictions', () => {
			it('can get global restrictions by one mosaic id, (GET)', () => {
				// Arrange:
				const req = { params: { mosaicId: testMosaicIds.one.id } };
				const route = mockServer.getRoute('/restrictions/mosaic/:mosaicId').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicRestrictionsByMosaicIdsFake.calledOnce).to.equal(true);
					expect(dbMosaicRestrictionsByMosaicIdsFake.firstCall.args[0]).to.deep.equal([testMosaicIds.one.uInt64]);
					expect(dbMosaicRestrictionsByMosaicIdsFake.firstCall.args[1]).to.deep.equal(
						restriction.mosaicRestriction.restrictionType.global
					);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: mosaicGlobalRestrictionEntrySample,
						type: 'mosaicRestriction.mosaicGlobalRestriction'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});

			it('can get global restrictions by several mosaic ids, (POST)', () => {
				// Arrange:
				const req = { params: { mosaicIds: [testMosaicIds.one.id, testMosaicIds.two.id] } };
				const route = mockServer.getRoute('/restrictions/mosaic').post();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(dbMosaicRestrictionsByMosaicIdsFake.calledOnce).to.equal(true);
					expect(dbMosaicRestrictionsByMosaicIdsFake.firstCall.args[0]).to.deep.equal([
						testMosaicIds.one.uInt64,
						testMosaicIds.two.uInt64
					]);
					expect(dbMosaicRestrictionsByMosaicIdsFake.firstCall.args[1]).to.deep.equal(
						restriction.mosaicRestriction.restrictionType.global
					);

					expect(mockServer.send.firstCall.args[0]).to.deep.equal({
						payload: [mosaicGlobalRestrictionEntrySample],
						type: 'mosaicRestriction.mosaicGlobalRestriction'
					});
					expect(mockServer.next.calledOnce).to.equal(true);
				});
			});
		});

		describe('mosaic address restrictions', () => {
			describe('can get mosaic address restrictions (GET)', () => {
				const route = mockServer.getRoute('/restrictions/mosaic/:mosaicId/address/:targetAddress').get();

				it('can get from address', () => {
					// Arrange:
					const req = { params: { mosaicId: testMosaicIds.one.id, targetAddress: testAddress } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbMosaicAddressRestrictionsFake.calledOnce).to.equal(true);
						expect(dbMosaicAddressRestrictionsFake.firstCall.args[0]).to.deep.equal(testMosaicIds.one.uInt64);
						expect(dbMosaicAddressRestrictionsFake.firstCall.args[1]).to.deep.equal([uint8TestAddress]);
					});
				});

				it('can get mosaic address restrictions', () => {
					// Arrange:
					const req = { params: { mosaicId: testMosaicIds.one.id, targetAddress: testAddress } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: mosaicAddressRestrictionEntrySample,
							type: 'mosaicRestriction.mosaicAddressRestriction'
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			});

			describe('can get mosaic address restrictions (POST)', () => {
				const route = mockServer.getRoute('/restrictions/mosaic/:mosaicId').post();

				it('can get from address', () => {
					// Arrange:
					const req = { params: { mosaicId: testMosaicIds.one.id, addresses: [testAddress, testAddress] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbMosaicAddressRestrictionsFake.calledOnce).to.equal(true);
						expect(dbMosaicAddressRestrictionsFake.firstCall.args[0]).to.deep.equal(testMosaicIds.one.uInt64);
						expect(dbMosaicAddressRestrictionsFake.firstCall.args[1]).to.deep.equal([uint8TestAddress, uint8TestAddress]);
					});
				});

				it('can get mosaic address restrictions', () => {
					// Arrange:
					const req = { params: { mosaicId: testMosaicIds.one.id, addresses: [testAddress, testAddress] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(mockServer.send.firstCall.args[0]).to.deep.equal({
							payload: [mosaicAddressRestrictionEntrySample],
							type: 'mosaicRestriction.mosaicAddressRestriction'
						});
						expect(mockServer.next.calledOnce).to.equal(true);
					});
				});
			});
		});
	});
});
