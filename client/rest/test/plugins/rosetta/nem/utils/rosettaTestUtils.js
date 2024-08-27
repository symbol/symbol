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

import NemProxy from '../../../../../src/plugins/rosetta/nem/NemProxy.js';
import {
	BasicFetchStubHelper,
	assertRosettaErrorRaisedBasicWithRegister,
	assertRosettaSuccessBasicWithRegister
} from '../../utils/rosettaTestUtils.js';

// region FetchStubHelper

export const FetchStubHelper = {
	...BasicFetchStubHelper,

	stubMosaicResolution: (namespaceId, name, divisibility, height = undefined) => {
		const heightQuery = undefined !== height ? `&height=${height}` : '';
		FetchStubHelper.stubPost(`local/mosaic/definition/supply?mosaicId=${namespaceId}:${name}${heightQuery}`, true, {
			data: [
				{
					mosaicDefinition: {
						id: { namespaceId, name },
						properties: [
							{ name: 'divisibility', value: divisibility.toString() }
						],
						levy: {}
					},
					supply: 88664422
				}
			]
		});
	},

	stubLocalBlockAt(optionsOrResponse, ok) {
		let response = optionsOrResponse;
		if (undefined === optionsOrResponse.block) {
			const block = { height: optionsOrResponse.height };
			if (optionsOrResponse.timestamp)
				block.timeStamp = optionsOrResponse.timestamp;

			// simulate NEM REST, which always returns lowercase hashes
			response = { block, hash: optionsOrResponse.hash.toLowerCase() };
		}

		FetchStubHelper.stubPost('local/block/at', ok, response, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: `{"height":${response.block.height}}`
		});
	}
};

// endregion

// region RosettaObjectFactory

export const RosettaObjectFactory = {
	createRosettaNetworkIdentifier: () => ({
		blockchain: 'NEM',
		network: 'testnet'
	}),

	createRosettaPublicKey: hexBytes => ({
		hex_bytes: hexBytes,
		curve_type: 'edwards25519_keccak'
	})
};

// endregion

// region asserts

const createRegisterRoutes = routes => server => {
	routes.register(server, {}, {
		config: {
			network: { name: 'testnet' }
		},
		proxy: new NemProxy('http://localhost:3456')
	});
};

export const assertRosettaErrorRaisedBasicWithRoutes = async (routes, ...args) => assertRosettaErrorRaisedBasicWithRegister(
	createRegisterRoutes(routes),
	...args
);

export const assertRosettaSuccessBasicWithRoutes = async (routes, ...args) => assertRosettaSuccessBasicWithRegister(
	createRegisterRoutes(routes),
	...args
);

// endregion
