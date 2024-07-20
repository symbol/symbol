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

import {
	FetchStubHelper, RosettaObjectFactory, assertRosettaErrorRaisedBasicWithRoutes, assertRosettaSuccessBasicWithRoutes
} from './utils/rosettaTestUtils.js';
import networkRoutes from '../../../src/plugins/rosetta/networkRoutes.js';
import Allow from '../../../src/plugins/rosetta/openApi/model/Allow.js';
import NetworkIdentifier from '../../../src/plugins/rosetta/openApi/model/NetworkIdentifier.js';
import NetworkListResponse from '../../../src/plugins/rosetta/openApi/model/NetworkListResponse.js';
import NetworkOptionsResponse from '../../../src/plugins/rosetta/openApi/model/NetworkOptionsResponse.js';
import OperationStatus from '../../../src/plugins/rosetta/openApi/model/OperationStatus.js';
import Version from '../../../src/plugins/rosetta/openApi/model/Version.js';
import { RosettaErrorFactory } from '../../../src/plugins/rosetta/rosettaUtils.js';

describe('network routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(networkRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(networkRoutes, ...args);

	// region type factories

	const { createRosettaNetworkIdentifier, createRosettaPublicKey } = RosettaObjectFactory;

	const createRosettaNodeVersion = () => ({
		version: 0x04050308
	});

	// endregion

	// region list

	describe('list', () => {
		it('can return current network', async () => {
			// Arrange: create expected response
			const expectedResponse = new NetworkListResponse([
				new NetworkIdentifier('Symbol', 'testnet')
			]);

			// Act + Assert:
			await assertRosettaSuccessBasic('/network/list', undefined, expectedResponse);
		});
	});

	// endregion

	// region options

	describe('options', () => {
		const stubFetchResult = FetchStubHelper.stubPost;
		FetchStubHelper.registerStubCleanup();

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			public_key: createRosettaPublicKey('E85D10BF47FFBCE2230F70CB43ED2DDE04FCF342524B383972F86EA1FF773C79')
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/network/options', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (node/info)', async () => {
			// Arrange:
			FetchStubHelper.stubCatapultProxyCacheFill();
			stubFetchResult('node/info', false, createRosettaNodeVersion());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('succeeds when all fetches succeed ', async () => {
			// Arrange:
			FetchStubHelper.stubCatapultProxyCacheFill();
			stubFetchResult('node/info', true, createRosettaNodeVersion());

			// - create expected response
			const version = new Version('1.4.13', '4.5.3.8');
			const allow = new Allow(
				[new OperationStatus('success', true)],
				['cosign', 'fee', 'multisig', 'transfer'],
				[
					RosettaErrorFactory.UNSUPPORTED_NETWORK,
					RosettaErrorFactory.UNSUPPORTED_CURVE,
					RosettaErrorFactory.INVALID_PUBLIC_KEY,
					RosettaErrorFactory.INVALID_REQUEST_DATA,
					RosettaErrorFactory.CONNECTION_ERROR,
					RosettaErrorFactory.SYNC_DURING_OPERATION,
					RosettaErrorFactory.NOT_SUPPORTED_ERROR,
					RosettaErrorFactory.INTERNAL_SERVER_ERROR
				].map(err => err.apiError),
				false,
				[],
				[],
				false
			);
			const expectedResponse = new NetworkOptionsResponse(version, allow);

			// Act + Assert:
			await assertRosettaSuccessBasic('/network/options', createValidRequest(), expectedResponse);
		});
	});

	// endregion
});
