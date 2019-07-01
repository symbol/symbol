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

const accountPropertiesRoutes = require('../../../src/plugins/accountProperties/accountPropertiesRoutes');
const catapult = require('catapult-sdk');
const { test } = require('../../routes/utils/routeTestUtils');

const { address, networkInfo } = catapult.model;
const { addresses, publicKeys } = test.sets;
const { convert } = catapult.utils;

const publicKeyToAddress = publicKey => address.publicKeyToAddress(convert.hexToUint8(publicKey), networkInfo.networks.mijinTest.id);

describe('account properties routes', () => {
	const config = { network: { name: 'mijinTest' } };

	describe('get by address', () => {
		test.route.document.addGetPostDocumentRouteTests(accountPropertiesRoutes.register, {
			routes: { singular: '/account/:accountId/properties', plural: '/account/properties' },
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
			dbApiName: 'accountPropertiesByAddresses',
			type: 'accountProperties',
			config
		});
	});

	describe('get by public key', () => {
		test.route.document.addGetPostDocumentRouteTests(accountPropertiesRoutes.register, {
			routes: { singular: '/account/:accountId/properties', plural: '/account/properties' },
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
			dbApiName: 'accountPropertiesByAddresses',
			type: 'accountProperties',
			config
		});
	});

	it('does not support publicKeys and addresses provided at the same time', () => {
		// Arrange:
		const keyGroups = [];
		const db = test.setup.createCapturingDb('accountPropertiesByAddresses', keyGroups, [{ value: 'this is nonsense' }]);

		// Act:
		const registerRoutes = accountPropertiesRoutes.register;
		const errorMessage = 'publicKeys and addresses cannot both be provided';
		return test.route.executeThrows(
			registerRoutes,
			'/account/properties',
			'post',
			{ addresses: addresses.valid, publicKeys: publicKeys.valid },
			db,
			{ transactionStates: [] },
			errorMessage,
			409
		);
	});
});
