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

const { expect } = require('chai');
const catapult = require('catapult-sdk');
const multisigRoutes = require('../../../src/plugins/routes/multisigRoutes');
const test = require('../../routes/utils/routeTestUtils');
const routeAccountIdGetTestUtils = require('./utils/routeAccountIdGetTestUtils');

const { convert } = catapult.utils;

describe('multisig routes', () => {
	describe('get by account', () => {
		routeAccountIdGetTestUtils.addDefaultTests({
			registerRoutes: multisigRoutes.register,
			route: '/account/:accountId/multisig',
			dbApiName: 'multisigsByAccounts',
			dbType: 'multisigEntry'
		});
	});

	describe('get multisig graph by account', () => {
		const createRouteDescriptor = routeAccountIdGetTestUtils.routeDescriptorFactory({
			registerRoutes: multisigRoutes.register,
			route: '/account/:accountId/multisig/graph',
			dbApiName: 'multisigsByAccounts',
			dbType: 'multisigGraph'
		});

		const addGetTests = dataTraits => {
			const routeDescriptor = createRouteDescriptor(dataTraits);
			routeDescriptor.extendDb = db => {
				(originalMultisigsByAccounts => {
					db.multisigsByAccounts = (...args) => originalMultisigsByAccounts(...args)
						.then(result => (undefined === result ? [] : result));
				})(db.multisigsByAccounts);
			};
			const getDocumentRouteTests = test.route.document.prepareGetDocumentRouteTests(multisigRoutes.register, routeDescriptor);
			getDocumentRouteTests.addNotFoundInputTest(routeDescriptor.inputs.valid);
			getDocumentRouteTests.addInvalidKeyTest(routeDescriptor.inputs.invalid);
		};

		// note: multisig/graph has complicated "valid" tests (below), so instead of using
		// addDefaultTests, use addGetDocumentTests to run only invalid tests.
		routeAccountIdGetTestUtils.addGetDocumentTests(addGetTests);

		const createMultisigEntry = (marker, upstreamCount, downstreamCount) => {
			const upstreamArray = [];
			for (let i = 0; i < upstreamCount; ++i) {
				const publicKey = test.random.publicKey();
				publicKey[0] = marker - 1;
				upstreamArray.push({ buffer: publicKey });
			}

			const downstreamArray = [];
			for (let i = 0; i < downstreamCount; ++i) {
				const publicKey = test.random.publicKey();
				publicKey[0] = marker + 1;
				downstreamArray.push({ buffer: publicKey });
			}

			return {
				multisig: {
					account: { buffer: test.random.publicKey() },
					multisigAccounts: upstreamArray,
					cosignatories: downstreamArray
				}
			};
		};

		const extractPublicKeys = (multisigEntryArray, fieldname) => {
			const publicKeys = [];
			multisigEntryArray.forEach(multisigEntry => multisigEntry.multisig[fieldname].forEach(publicKey => {
				publicKeys.push(publicKey.buffer);
			}));

			return publicKeys;
		};

		const createPublicKeyWithMarker = marker => {
			const publicKey = new Uint8Array(test.random.publicKey());
			publicKey[0] = marker;
			return publicKey;
		};

		const runTest = (publicKeyParam, multisigEntriesArray, expectedParams) => {
			// Arrange:
			// Note that the first byte of each public key is used as index into the  multisigEntriesArray
			const multisigsByAccountsParams = [];
			const db = {
				multisigsByAccounts: (type, accountIds) => {
					multisigsByAccountsParams.push(accountIds);
					if (0 === accountIds.length)
						return Promise.resolve([]);

					return Promise.resolve(multisigEntriesArray[accountIds[0][0]]);
				}
			};

			// Act:
			return test.route.executeSingle(
				multisigRoutes.register,
				'/account/:accountId/multisig/graph',
				'get',
				{ accountId: convert.uint8ToHex(publicKeyParam) },
				db,
				undefined,
				response => {
					// Assert: parameters passed to db functions are correct
					// note that sort is needed since the upstream and downstream operations run concurrently resulting in
					// indeterminate call order of the db function
					expect(multisigsByAccountsParams.sort()).to.deep.equal(expectedParams.sort());

					// check response (publicKeyParam[0] is index, which is negative of level)
					let level = 0 - publicKeyParam[0];
					const expectedPayload = multisigEntriesArray.map(multisigEntries => ({ level: level++, multisigEntries }));

					expect(response).to.deep.equal({ payload: expectedPayload, type: 'multisigGraph' });
				}
			);
		};

		it('returns correct graph if multisig account has neither upstream nor downstream accounts', () => {
			// Arrange:
			const multisigEntriesArray = [[createMultisigEntry(0, 0, 0)]];
			const publicKey = createPublicKeyWithMarker(0);
			const expectedParams = [[publicKey], [], []];

			// Act + Assert:
			return runTest(publicKey, multisigEntriesArray, expectedParams);
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
			const publicKey = createPublicKeyWithMarker(2);
			const expectedParams = [
				[publicKey],
				extractPublicKeys(multisigEntriesArray[2], 'multisigAccounts'),
				extractPublicKeys(multisigEntriesArray[1], 'multisigAccounts'),
				extractPublicKeys(multisigEntriesArray[0], 'multisigAccounts'),
				[]];

			// Act + Assert:
			return runTest(publicKey, multisigEntriesArray, expectedParams);
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
			const publicKey = createPublicKeyWithMarker(0);
			const expectedParams = [
				[publicKey],
				[],
				extractPublicKeys(multisigEntriesArray[0], 'cosignatories'),
				extractPublicKeys(multisigEntriesArray[1], 'cosignatories'),
				extractPublicKeys(multisigEntriesArray[2], 'cosignatories')];

			// Act + Assert:
			return runTest(publicKey, multisigEntriesArray, expectedParams);
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
			const publicKey = createPublicKeyWithMarker(2);
			const expectedParams = [
				[publicKey],
				extractPublicKeys(multisigEntriesArray[2], 'multisigAccounts'),
				extractPublicKeys(multisigEntriesArray[1], 'multisigAccounts'),
				[],
				extractPublicKeys(multisigEntriesArray[2], 'cosignatories'),
				[]];

			// Act + Assert:
			return runTest(publicKey, multisigEntriesArray, expectedParams);
		});
	});
});
