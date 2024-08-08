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
import networkRoutes from '../../../../src/plugins/rosetta/nem/networkRoutes.js';
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
import { NetworkLocator } from 'symbol-sdk';
import { Network } from 'symbol-sdk/nem';

describe('NEM network routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(networkRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(networkRoutes, ...args);

	// region type factories

	const { createRosettaNetworkIdentifier, createRosettaPublicKey } = RosettaObjectFactory;

	const createRosettaNodeVersion = (features = 1) => ({
		metaData: { version: '4.5.3.8', features }
	});

	// endregion

	// region list

	describe('list', () => {
		it('can return current network', async () => {
			// Arrange: create expected response
			const expectedResponse = new NetworkListResponse([
				new NetworkIdentifier('NEM', 'testnet')
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
			stubFetchResult('node/info', false, createRosettaNodeVersion());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		const assertRosettaSuccessWithHistoricalNode = async historicalNodeEnable => {
			// Arrange:
			stubFetchResult('node/info', true, createRosettaNodeVersion(historicalNodeEnable ? 3 : 1));

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
				historicalNodeEnable,
				[],
				[],
				false
			);
			const expectedResponse = new NetworkOptionsResponse(version, allow);

			// Act + Assert:
			await assertRosettaSuccessBasic('/network/options', createValidRequest(), expectedResponse);
		};

		it('succeeds when all fetches succeed', async () => assertRosettaSuccessWithHistoricalNode(false));

		it('succeeds when all fetches succeed on historical node', async () => assertRosettaSuccessWithHistoricalNode(true));
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
		const NODE_PEERS_PUBLIC_KEYS = [
			'CE344133E42F557D6A87C2EBA8E9AB44C225A80E713A593CE39D58FB70AA83F4',
			'AE3C8C118ECB82333BAAFD5BE858176E3C0A497CDA405ACCBA6F737E1C443D2D',
			'CC1287250B978C0638FD0461EB86952BAEAB4F04266A09FDB3D96D5412BD5B57'
		];

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier()
		});

		const createBlocksResponse = (blockHeight, blockHash, blockTimestamp) => ({
			hash: blockHash,
			block: {
				height: blockHeight,
				timeStamp: blockTimestamp
			}
		});

		const createNodePeersResponse = () => {
			const peers = NODE_PEERS_PUBLIC_KEYS.map(publicKey => ({ identity: { 'public-key': publicKey } }));
			return { data: peers };
		};

		const setupEndPoints = success => {
			FetchStubHelper.stubPost('node/info', success, { metaData: { version: '4.5.3.8' } });
			FetchStubHelper.stubPost('local/block/at', success, { hash: GENESIS_BLOCK_HASH, block: { height: GENESIS_BLOCK_NUMBER } }, {
				method: 'POST',
				body: JSON.stringify({ height: GENESIS_BLOCK_NUMBER }),
				headers: { 'Content-Type': 'application/json' }
			});
			FetchStubHelper.stubPost(
				'chain/last-block',
				success,
				createBlocksResponse(CURRENT_BLOCK_NUMBER, CURRENT_BLOCK_HASH, CURRENT_BLOCK_TIMESTAMP)
			);
			stubFetchResult('node/peer-list/reachable', success, createNodePeersResponse());
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
			stubFetchResult('node/peer-list/reachable', false, createNodePeersResponse());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when one fetch fails (node/peer-list/reachable)', async () => {
			// Arrange:
			setupEndPoints(true);
			stubFetchResult('node/peer-list/reachable', false, createNodePeersResponse());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when node/peer-list/reachable returns invalid json', async () => {
			// Arrange:
			setupEndPoints(true);
			stubFetchResult('node/peer-list/reachable', true, ({ data: [{ wrongIdentity: {} }] }));

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.INTERNAL_SERVER_ERROR, () => {});
		});

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			setupEndPoints(true);
			const network = NetworkLocator.findByName(Network.NETWORKS, 'testnet');

			// - create expected response
			const expectedResponse = new NetworkStatusResponse();
			expectedResponse.genesis_block_identifier = new BlockIdentifier(GENESIS_BLOCK_NUMBER, GENESIS_BLOCK_HASH);
			expectedResponse.current_block_identifier = new BlockIdentifier(CURRENT_BLOCK_NUMBER, CURRENT_BLOCK_HASH);
			expectedResponse.current_block_timestamp = Number(network.datetimeConverter.toDatetime(CURRENT_BLOCK_TIMESTAMP).getTime());
			expectedResponse.peers = NODE_PEERS_PUBLIC_KEYS.map(publicKey => new Peer(publicKey));

			// Act + Assert:
			await assertRosettaSuccessBasic('/network/status', createValidRequest(), expectedResponse);
		});
	});

	// endregion
});
