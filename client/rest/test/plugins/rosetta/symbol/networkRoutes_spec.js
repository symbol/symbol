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
import Allow from '../../../../src/plugins/rosetta/openApi/model/Allow.js';
import BlockIdentifier from '../../../../src/plugins/rosetta/openApi/model/BlockIdentifier.js';
import NetworkIdentifier from '../../../../src/plugins/rosetta/openApi/model/NetworkIdentifier.js';
import NetworkListResponse from '../../../../src/plugins/rosetta/openApi/model/NetworkListResponse.js';
import NetworkOptionsResponse from '../../../../src/plugins/rosetta/openApi/model/NetworkOptionsResponse.js';
import NetworkStatusResponse from '../../../../src/plugins/rosetta/openApi/model/NetworkStatusResponse.js';
import OperationStatus from '../../../../src/plugins/rosetta/openApi/model/OperationStatus.js';
import Peer from '../../../../src/plugins/rosetta/openApi/model/Peer.js';
import Version from '../../../../src/plugins/rosetta/openApi/model/Version.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import networkRoutes from '../../../../src/plugins/rosetta/symbol/networkRoutes.js';
import { expect } from 'chai';

describe('Symbol rosetta network routes', () => {
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

	// region status

	describe('status', () => {
		const stubFetchResult = FetchStubHelper.stubPost;
		FetchStubHelper.registerStubCleanup();

		const GENESIS_BLOCK_HASH = '135B9276FE14D6724F87764DD3F8E332813AE9A36D1A7953B12213EA9ACB6B97';
		const GENESIS_BLOCK_NUMBER = 1;
		const CURRENT_BLOCK_HASH = '92435746EFD1789E5E906E99B8E3BDC72AB1ADECDBEECB321116E59DED0A739C';
		const CURRENT_BLOCK_NUMBER = 10;
		const CURRENT_BLOCK_TIMESTAMP = 31078501;
		const EPOCH_ADJUSTMENT = '1667250467s';
		const NODE_PEERS_PUBLIC_KEYS = [
			'CE344133E42F557D6A87C2EBA8E9AB44C225A80E713A593CE39D58FB70AA83F4',
			'AE3C8C118ECB82333BAAFD5BE858176E3C0A497CDA405ACCBA6F737E1C443D2D',
			'CC1287250B978C0638FD0461EB86952BAEAB4F04266A09FDB3D96D5412BD5B57'
		];

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier()
		});

		const createBlocksResponse = (blockHeight, blockHash, blockTimestamp) => ({
			meta: { hash: blockHash },
			block: {
				height: blockHeight,
				timestamp: blockTimestamp
			}
		});

		const createChainInfoResponse = () => ({
			height: CURRENT_BLOCK_NUMBER
		});

		const createNetworkPropertiesResponse = () => ({
			network: { epochAdjustment: EPOCH_ADJUSTMENT }
		});

		const createNodePeersResponse = () => NODE_PEERS_PUBLIC_KEYS.map(publicKey => ({ publicKey }));

		const setupEndPoints = success => {
			stubFetchResult('node/info', success, createRosettaNodeVersion());
			stubFetchResult(`blocks/${GENESIS_BLOCK_NUMBER}`, success, createBlocksResponse(GENESIS_BLOCK_NUMBER, GENESIS_BLOCK_HASH, 0));
			stubFetchResult(
				`blocks/${CURRENT_BLOCK_NUMBER}`,
				success,
				createBlocksResponse(CURRENT_BLOCK_NUMBER, CURRENT_BLOCK_HASH, CURRENT_BLOCK_TIMESTAMP)
			);
			stubFetchResult('chain/info', success, createChainInfoResponse());
			stubFetchResult('network/properties', success, createNetworkPropertiesResponse());
			stubFetchResult('node/peers', success, createNodePeersResponse());

			// currencyMosaicId caching
			stubFetchResult('namespaces/E74B99BA41F4AFEE', true, { namespace: { alias: { mosaicId: '1122334455667788' } } });
			FetchStubHelper.stubMosaicResolution('1122334455667788', 3, 'foo.bar');
		};

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/network/status', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when all fetch fails', async () => {
			// Arrange:
			setupEndPoints(false);

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when multiple fetch fails', async () => {
			// Arrange:
			setupEndPoints(true);
			stubFetchResult(
				`blocks/${CURRENT_BLOCK_NUMBER}`,
				false,
				createBlocksResponse(CURRENT_BLOCK_NUMBER, CURRENT_BLOCK_HASH, CURRENT_BLOCK_TIMESTAMP)
			);
			stubFetchResult('node/peers', false, createNodePeersResponse());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when one fetch fails (node/peers)', async () => {
			// Arrange:
			setupEndPoints(true);
			stubFetchResult('node/peers', false, createNodePeersResponse());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when node/peers returns invalid json', async () => {
			// Arrange:
			setupEndPoints(true);
			stubFetchResult('node/peers', true, undefined);

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.INTERNAL_SERVER_ERROR, () => {});
		});

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			setupEndPoints(true);

			// - create expected response
			const expectedResponse = new NetworkStatusResponse();
			expectedResponse.genesis_block_identifier = new BlockIdentifier(GENESIS_BLOCK_NUMBER, GENESIS_BLOCK_HASH);
			expectedResponse.current_block_identifier = new BlockIdentifier(CURRENT_BLOCK_NUMBER, CURRENT_BLOCK_HASH);
			expectedResponse.current_block_timestamp = (1667250467 * 1000) + CURRENT_BLOCK_TIMESTAMP;
			expectedResponse.peers = NODE_PEERS_PUBLIC_KEYS.map(publicKey => new Peer(publicKey));

			// Act + Assert:
			await assertRosettaSuccessBasic('/network/status', createValidRequest(), expectedResponse);

			// - cache routes should only be queried once
			expect(global.fetch.withArgs('http://localhost:3456/node/info').callCount).to.equal(1);
			expect(global.fetch.withArgs('http://localhost:3456/namespaces/E74B99BA41F4AFEE').callCount).to.equal(1);
			expect(global.fetch.withArgs('http://localhost:3456/mosaics/1122334455667788').callCount).to.equal(1);
		});
	});

	// endregion
});
