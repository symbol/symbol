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

const catapult = require('catapult-sdk');
const test = require('../../../routes/utils/routeTestUtils');

const Valid_Address = test.sets.addresses.valid[0];
const Valid_Public_Key = test.sets.publicKeys.valid[0];

const routeAccountIdGetTestUtils = {
	routeDescriptorFactory: traits =>
		dataTraits => ({
			route: traits.route,
			inputs: {
				valid: {
					object: { accountId: dataTraits.valid },
					parsed: dataTraits.expected,
					printable: dataTraits.valid
				},
				invalid: { object: { accountId: '12345' }, error: 'accountId has an invalid format' }
			},
			dbApiName: traits.dbApiName,
			type: traits.dbType
		}),

	addGetDocumentTests: addTests => {
		const Valid_Address_Traits = {
			valid: Valid_Address,
			expected: ['address', [catapult.model.address.stringToAddress(Valid_Address)]]
		};

		const Valid_Public_Key_Traits = {
			valid: Valid_Public_Key,
			expected: ['publicKey', [catapult.utils.convert.hexToUint8(Valid_Public_Key)]]
		};

		describe('by address', () => addTests(Valid_Address_Traits));
		describe('by publicKey', () => addTests(Valid_Public_Key_Traits));
	},

	addDefaultTests: traits => {
		const createRouteDescriptor = routeAccountIdGetTestUtils.routeDescriptorFactory(traits);
		const addGetTests = dataTraits => {
			const routeDescriptor = createRouteDescriptor(dataTraits);

			// Assert:
			test.route.document.addGetDocumentRouteTests(traits.registerRoutes, routeDescriptor);
		};

		routeAccountIdGetTestUtils.addGetDocumentTests(addGetTests);
	}
};

module.exports = routeAccountIdGetTestUtils;
