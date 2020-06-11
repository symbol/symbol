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

const lockSecretRoutes = require('../../../src/plugins/lockSecret/lockSecretRoutes');
const { test } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');

const { addresses } = test.sets;
const { address } = catapult.model;
const { convert } = catapult.utils;

describe('lock secret routes', () => {
	const factory = {
		createLockSecretPagingRouteInfo: (routeName, routeCaptureMethod, dbMethod) => ({
			routes: lockSecretRoutes,
			routeName,
			createDb: (keyGroups, documents) => ({
				[dbMethod]: (accountAddresses, pageId, pageSize) => {
					keyGroups.push({
						accountAddresses, pageId, pageSize
					});
					return Promise.resolve(documents);
				}
			}),
			routeCaptureMethod
		})
	};

	describe('get secret lock infos by address', () => {
		describe('get by address', () => {
			const addGetTests = traits => {
				const pagingTestsFactory = test.setup.createPagingTestsFactory(
					factory.createLockSecretPagingRouteInfo('/account/:address/lock/secret', 'get', 'secretLocksByAddresses'),
					traits.valid.params,
					traits.valid.expected,
					'secretLockInfo'
				);

				pagingTestsFactory.addDefault();
				pagingTestsFactory.addNonPagingParamFailureTest('address', traits.invalid.addresses);
			};

			describe('by address', () => addGetTests({
				valid: {
					params: { address: addresses.valid[0] },
					expected: { accountAddresses: [address.stringToAddress(addresses.valid[0])] }
				},
				invalid: {
					addresses: addresses.invalid,
					error: 'illegal base32 character 1'
				}
			}));
		});
	});

	describe('get secret lock info by hash', () => {
		const secret = '5994471ABB01112AFCC18159F6CC74B4F511B99806DA59B3CAF5A9C173CACFC5';
		test.route.document.addGetDocumentRouteTests(lockSecretRoutes.register, {
			route: '/lock/secret/:secret',
			inputs: {
				valid: { object: { secret }, parsed: [convert.hexToUint8(secret)], printable: secret },
				invalid: {
					object: { secret: '12345' },
					error: 'secret has an invalid format'
				}
			},
			dbApiName: 'secretLockBySecret',
			type: 'secretLockInfo'
		});
	});
});
