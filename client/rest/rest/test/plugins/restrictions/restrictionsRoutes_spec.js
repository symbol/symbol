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

const { address, networkInfo, mosaicRestriction } = catapult.model;
const { addresses, publicKeys } = test.sets;
const { convert } = catapult.utils;

describe('restrictions routes', () => {
	describe('account restrictions', () => {
		const publicKeyToAddress = publicKey =>
			address.publicKeyToAddress(convert.hexToUint8(publicKey), networkInfo.networks.mijinTest.id);

		const config = { network: { name: 'mijinTest' } };

		describe('get by address', () => {
			test.route.document.addGetPostDocumentRouteTests(restrictionsRoutes.register, {
				routes: { singular: '/restrictions/account/:accountId', plural: '/restrictions/account' },
				inputs: {
					valid: {
						object: { accountId: addresses.valid[0] },
						parsed: [address.stringToAddress(addresses.valid[0])],
						printable: addresses.valid[0]
					},
					validMultiple: {
						object: { addresses: addresses.valid },
						parsed: addresses.valid.map(address.stringToAddress)
					},
					invalid: {
						object: { accountId: addresses.invalid },
						error: 'accountId has an invalid format'
					},
					invalidMultiple: {
						object: { addresses: [addresses.valid[0], '12345', addresses.valid[1]] },
						error: 'element in array addresses has an invalid format'
					}
				},
				dbApiName: 'accountRestrictionsByAddresses',
				type: 'accountRestrictions',
				config
			});
		});

		describe('get by public key', () => {
			test.route.document.addGetPostDocumentRouteTests(restrictionsRoutes.register, {
				routes: { singular: '/restrictions/account/:accountId', plural: '/restrictions/account' },
				inputs: {
					valid: {
						object: { accountId: publicKeys.valid[0] },
						parsed: [publicKeyToAddress(publicKeys.valid[0])],
						printable: publicKeys.valid[0]
					},
					validMultiple: {
						object: { publicKeys: publicKeys.valid },
						parsed: publicKeys.valid.map(publicKey =>
							publicKeyToAddress(publicKey))
					},
					invalid: {
						object: { accountId: publicKeys.invalid },
						error: 'accountId has an invalid format'
					},
					invalidMultiple: {
						object: { publicKeys: [publicKeys.valid[0], '12345', publicKeys.valid[1]] },
						error: 'element in array publicKeys has an invalid format'
					}
				},
				dbApiName: 'accountRestrictionsByAddresses',
				type: 'accountRestrictions',
				config
			});
		});

		it('does not support publicKeys and addresses provided at the same time', () => {
			// Arrange:
			const keyGroups = [];
			const db = test.setup.createCapturingDb('accountRestrictionsByAddresses', keyGroups, [{ value: 'this is nonsense' }]);

			// Act:
			const registerRoutes = restrictionsRoutes.register;
			const errorMessage = 'publicKeys and addresses cannot both be provided';
			return test.route.executeThrows(
				registerRoutes,
				'/restrictions/account',
				'post',
				{ addresses: addresses.valid, publicKeys: publicKeys.valid },
				db,
				{ transactionStates: [] },
				errorMessage,
				409
			);
		});
	});

	describe('mosaic restrictions', () => {
		const testPublicKey = '7DE16AEDF57EB9561D3E6EFA4AE66F27ABDA8AEC8BC020B6277360E31619DCE7';

		const testAddress = 'SBZ22LWA7GDZLPLQF7PXTMNLWSEZ7ZRVGRMWLXWV';
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
					expect(dbMosaicRestrictionsByMosaicIdsFake.firstCall.args[1]).to.deep.equal(mosaicRestriction.restrictionType.global);

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
					expect(dbMosaicRestrictionsByMosaicIdsFake.firstCall.args[1]).to.deep.equal(mosaicRestriction.restrictionType.global);

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
				const route = mockServer.getRoute('/restrictions/mosaic/:mosaicId/address/:accountId').get();

				it('can get from publicKey', () => {
					// Arrange:
					const req = { params: { mosaicId: testMosaicIds.one.id, accountId: testPublicKey } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbMosaicAddressRestrictionsFake.calledOnce).to.equal(true);
						expect(dbMosaicAddressRestrictionsFake.firstCall.args[0]).to.deep.equal(testMosaicIds.one.uInt64);
						expect(dbMosaicAddressRestrictionsFake.firstCall.args[1]).to.deep.equal([uint8TestAddress]);
					});
				});

				it('can get from address', () => {
					// Arrange:
					const req = { params: { mosaicId: testMosaicIds.one.id, accountId: testAddress } };

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
					const req = { params: { mosaicId: testMosaicIds.one.id, accountId: testPublicKey } };

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

				it('can get from publicKey', () => {
					// Arrange:
					const req = { params: { mosaicId: testMosaicIds.one.id, publicKeys: [testPublicKey, testPublicKey] } };

					// Act:
					return mockServer.callRoute(route, req).then(() => {
						// Assert:
						expect(dbMosaicAddressRestrictionsFake.calledOnce).to.equal(true);
						expect(dbMosaicAddressRestrictionsFake.firstCall.args[0]).to.deep.equal(testMosaicIds.one.uInt64);
						expect(dbMosaicAddressRestrictionsFake.firstCall.args[1]).to.deep.equal([uint8TestAddress, uint8TestAddress]);
					});
				});

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
					const req = { params: { mosaicId: testMosaicIds.one.id, publicKeys: [testPublicKey, testPublicKey] } };

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

			describe('does not support publicKeys and addresses provided at the same time', () => {
				it('does not support publicKeys and addresses provided at the same time', () => {
					// Arrange:
					const req = { params: { mosaicId: testMosaicIds.one.id, publicKeys: [''], addresses: [''] } };
					const route = mockServer.getRoute('/restrictions/mosaic/:mosaicId').post();

					// Act + Assert:
					expect(() => mockServer.callRoute(route, req)).to.throw('publicKeys and addresses cannot both be provided');
				});
			});
		});
	});
});
