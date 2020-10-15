/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
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

const routeAddressGetTestUtils = require('./routeAddressGetTestUtils');
const multisigRoutes = require('../../../src/plugins/multisig/multisigRoutes');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');

describe('multisig routes', () => {
	describe('get by account', () => {
		routeAddressGetTestUtils.addDefaultTests({
			registerRoutes: multisigRoutes.register,
			route: '/account/:address/multisig',
			dbApiName: 'multisigsByAddresses',
			dbType: 'multisigEntry'
		});
	});

	describe('get multisig graph by account', () => {
		const createRouteDescriptor = routeAddressGetTestUtils.routeDescriptorFactory({
			registerRoutes: multisigRoutes.register,
			route: '/account/:address/multisig/graph',
			dbApiName: 'multisigsByAddresses',
			dbType: 'multisigGraph'
		});

		const addGetTests = dataTraits => {
			const routeDescriptor = createRouteDescriptor(dataTraits);
			routeDescriptor.extendDb = db => {
				(originalMultisigsByAddresses => {
					db.multisigsByAddresses = (...args) => originalMultisigsByAddresses(...args)
						.then(result => (undefined === result ? [] : result));
				})(db.multisigsByAddresses);
			};
			const getDocumentRouteTests = test.route.document.prepareGetDocumentRouteTests(multisigRoutes.register, routeDescriptor);
			getDocumentRouteTests.addNotFoundInputTest(routeDescriptor.inputs.valid);
			getDocumentRouteTests.addInvalidKeyTest(routeDescriptor.inputs.invalid);
		};

		// note: multisig/graph has complicated "valid" tests (below), so instead of using
		// addDefaultTests, use addGetDocumentTests to run only invalid tests.
		routeAddressGetTestUtils.addGetDocumentTests(addGetTests);

		const createMultisigEntry = (marker, upstreamCount, downstreamCount) => {
			const upstreamArray = [];
			for (let i = 0; i < upstreamCount; ++i) {
				const address = test.random.address();
				address[0] = marker - 1;
				upstreamArray.push({ buffer: address });
			}

			const downstreamArray = [];
			for (let i = 0; i < downstreamCount; ++i) {
				const address = test.random.address();
				address[0] = marker + 1;
				downstreamArray.push({ buffer: address });
			}

			return {
				multisig: {
					accountAddress: { buffer: test.random.address() },
					multisigAddresses: upstreamArray,
					cosignatoryAddresses: downstreamArray
				}
			};
		};

		const extractAddresses = (multisigEntryArray, fieldname) => {
			const addresses = [];
			multisigEntryArray.forEach(multisigEntry => multisigEntry.multisig[fieldname].forEach(address => {
				addresses.push(address.buffer);
			}));

			return addresses;
		};

		const createAddressWithMarker = marker => {
			const address = new Uint8Array(test.random.address());
			address[0] = marker;
			return address;
		};

		const runTest = (accountAddress, multisigEntriesArray, expectedParams) => {
			// Arrange:
			// Note that the first byte of each address is used as index into the multisigEntriesArray
			const multisigsByAddressesParams = [];
			const db = {
				multisigsByAddresses: addresses => {
					multisigsByAddressesParams.push(addresses);
					if (0 === addresses.length)
						return Promise.resolve([]);

					return Promise.resolve(multisigEntriesArray[addresses[0][0]]);
				}
			};

			// Act:
			return test.route.executeSingle(
				multisigRoutes.register,
				'/account/:address/multisig/graph',
				'get',
				{ address: catapult.model.address.addressToString(accountAddress) },

				db,
				undefined,
				response => {
					// Assert: parameters passed to db functions are correct
					// note that sort is needed since the upstream and downstream operations run concurrently resulting in
					// indeterminate call order of the db function
					expect(multisigsByAddressesParams.sort()).to.deep.equal(expectedParams.sort());

					// check response (accountAddress[0] is index, which is negative of level)
					let level = 0 - accountAddress[0];
					const expectedPayload = multisigEntriesArray.map(multisigEntries => ({ level: level++, multisigEntries }));

					expect(response).to.deep.equal({ payload: expectedPayload, type: 'multisigGraph' });
				}
			);
		};

		it('returns correct graph if multisig account has neither upstream nor downstream accounts', () => {
			// Arrange:
			const multisigEntriesArray = [[createMultisigEntry(0, 0, 0)]];
			const address = createAddressWithMarker(0);
			const expectedParams = [[address], [], []];

			// Act + Assert:
			return runTest(address, multisigEntriesArray, expectedParams);
		});

		it('returns correct graph if multisig account has only upstream accounts', () => {
			// Arrange:
			// A1 - B1
			//        \
			// A2 - B2- C
			const multisigEntriesArray = [
				[createMultisigEntry(0, 0, 1), createMultisigEntry(0, 0, 1)],
				[createMultisigEntry(1, 1, 1), createMultisigEntry(1, 1, 1)],
				[createMultisigEntry(2, 2, 0)]
			];
			const address = createAddressWithMarker(2);
			const expectedParams = [
				[address],
				extractAddresses(multisigEntriesArray[2], 'multisigAddresses'),
				extractAddresses(multisigEntriesArray[1], 'multisigAddresses'),
				extractAddresses(multisigEntriesArray[0], 'multisigAddresses'),
				[]];

			// Act + Assert:
			return runTest(address, multisigEntriesArray, expectedParams);
		});

		it('returns correct graph if multisig account has only downstream accounts', () => {
			// Arrange:
			//    D1 - E1
			//   /
			// C - D2 - E2
			const multisigEntriesArray = [
				[createMultisigEntry(0, 0, 2)],
				[createMultisigEntry(1, 1, 1), createMultisigEntry(1, 1, 1)],
				[createMultisigEntry(1, 1, 0), createMultisigEntry(1, 1, 0)]
			];
			const address = createAddressWithMarker(0);
			const expectedParams = [
				[address],
				[],
				extractAddresses(multisigEntriesArray[0], 'cosignatoryAddresses'),
				extractAddresses(multisigEntriesArray[1], 'cosignatoryAddresses'),
				extractAddresses(multisigEntriesArray[2], 'cosignatoryAddresses')];

			// Act + Assert:
			return runTest(address, multisigEntriesArray, expectedParams);
		});

		it('returns correct graph if multisig account has upstream and downstream accounts', () => {
			// Arrange:
			//     B1      D1
			//   /   \   /
			// A - B2- C - D2
			const multisigEntriesArray = [
				[createMultisigEntry(0, 0, 2)],
				[createMultisigEntry(1, 1, 1), createMultisigEntry(1, 1, 1)],
				[createMultisigEntry(2, 2, 2)],
				[createMultisigEntry(3, 1, 0), createMultisigEntry(3, 1, 0)]
			];
			const address = createAddressWithMarker(2);
			const expectedParams = [
				[address],
				extractAddresses(multisigEntriesArray[2], 'multisigAddresses'),
				extractAddresses(multisigEntriesArray[1], 'multisigAddresses'),
				[],
				extractAddresses(multisigEntriesArray[2], 'cosignatoryAddresses'),
				[]];

			// Act + Assert:
			return runTest(address, multisigEntriesArray, expectedParams);
		});
	});
});
