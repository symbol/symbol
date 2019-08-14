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

const lockHashRoutes = require('../../../src/plugins/lockHash/lockHashRoutes');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');

const { addresses, publicKeys } = test.sets;
const { address } = catapult.model;
const { convert } = catapult.utils;

describe('lock hash routes', () => {
	const factory = {
		createLockHashPagingRouteInfo: (routeName, routeCaptureMethod, dbMethod) => ({
			routes: lockHashRoutes,
			routeName,
			createDb: (keyGroups, documents) => ({
				[dbMethod]: (type, accountIds, pageId, pageSize) => {
					keyGroups.push({
						type, accountIds, pageId, pageSize
					});
					return Promise.resolve(documents);
				}
			}),
			routeCaptureMethod
		})
	};

	describe('get hash lock infos', () => {
		describe('get by account', () => {
			const addGetTests = traits => {
				const pagingTestsFactory = test.setup.createPagingTestsFactory(
					factory.createLockHashPagingRouteInfo('/account/:accountId/lock/hash', 'get', 'hashLocksByAccounts'),
					traits.valid.params,
					traits.valid.expected,
					'hashLockInfo'
				);

				pagingTestsFactory.addDefault();
				pagingTestsFactory.addNonPagingParamFailureTest('accountId', traits.invalid.accountId);
			};

			describe('by address', () => addGetTests({
				valid: {
					params: { accountId: addresses.valid[0] },
					expected: { type: 'address', accountIds: [address.stringToAddress(addresses.valid[0])] }
				},
				invalid: {
					accountId: addresses.invalid,
					error: 'illegal base32 character 1'
				}
			}));

			describe('by publicKey', () => addGetTests({
				valid: {
					params: { accountId: publicKeys.valid[0] },
					expected: { type: 'publicKey', accountIds: [convert.hexToUint8(publicKeys.valid[0])] }
				},
				invalid: {
					accountId: publicKeys.invalid,
					error: 'unrecognized hex char \'1G\''
				}
			}));
		});
	});

	describe('get hash lock info by hash', () => {
		const hash = 'C54AFD996DF1F52748EBC5B40F8D0DC242A6A661299149F5F96A0C21ECCB653F';
		test.route.document.addGetDocumentRouteTests(lockHashRoutes.register, {
			route: '/lock/hash/:hash',
			inputs: {
				valid: { object: { hash }, parsed: [convert.hexToUint8(hash)], printable: hash },
				invalid: {
					object: { hash: '12345' },
					error: 'hash has an invalid format'
				}
			},
			dbApiName: 'hashLockByHash',
			type: 'hashLockInfo'
		});
	});
});
