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

import constructionRoutes from '../../../src/plugins/rosetta/constructionRoutes.js';
import AccountIdentifier from '../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import ConstructionDeriveResponse from '../../../src/plugins/rosetta/openApi/model/ConstructionDeriveResponse.js';
import ConstructionMetadataResponse from '../../../src/plugins/rosetta/openApi/model/ConstructionMetadataResponse.js';
import ConstructionPreprocessResponse from '../../../src/plugins/rosetta/openApi/model/ConstructionPreprocessResponse.js';
import RosettaError from '../../../src/plugins/rosetta/openApi/model/Error.js';
import { errors } from '../../../src/plugins/rosetta/rosettaUtils.js';
import MockServer from '../../routes/utils/MockServer.js';
import { expect } from 'chai';
import sinon from 'sinon';

describe('construction routes', () => {
	const createMockServer = () => {
		const mockServer = new MockServer();
		constructionRoutes.register(mockServer.server, {}, {
			config: {
				network: { name: 'testnet' },
				rest: { protocol: 'http', port: '3456' }
			}
		});
		return mockServer;
	};

	// region asserts

	const assertRosettaErrorRaisedBasic = async (routeName, request, expectedError, malformRequest) => {
		// Arrange:
		const mockServer = createMockServer();
		const route = mockServer.getRoute(routeName).post();

		malformRequest(request);

		// Act:
		await mockServer.callRoute(route, { body: request });

		// Assert:
		expect(mockServer.send.calledOnce).to.equal(true);
		expect(mockServer.next.calledOnce).to.equal(true);
		expect(mockServer.res.statusCode).to.equal(500);

		const response = mockServer.send.firstCall.args[0];
		expect(response).to.deep.equal(expectedError);
		expect(() => RosettaError.validateJSON(response)).to.not.throw();
	};

	const assertRosettaSuccessBasic = async (routeName, request, expectedResponse) => {
		// Arrange:
		const mockServer = createMockServer();
		const route = mockServer.getRoute(routeName).post();

		// Act:
		await mockServer.callRoute(route, { body: request });

		// Assert:
		expect(mockServer.send.calledOnce).to.equal(true);
		expect(mockServer.next.calledOnce).to.equal(true);
		expect(mockServer.res.statusCode).to.equal(undefined);

		const response = mockServer.send.firstCall.args[0];
		expect(response).to.deep.equal(expectedResponse);
		expect(() => expectedResponse.constructor.validateJSON(response)).to.not.throw();
	};

	// endregion

	// region type factories

	const createRosettaNetworkIdentifier = () => ({
		blockchain: 'Symbol',
		network: 'testnet'
	});

	const createRosettaCurrency = () => ({
		symbol: 'xym',
		decimals: 6
	});

	const createRosettaTransfer = (index, address, amount) => ({
		operation_identifier: { index },
		type: 'transfer',
		account: { address },
		amount: {
			value: amount,
			currency: createRosettaCurrency()
		}
	});

	const createRosettaMultisig = (index, address, metadata) => ({
		operation_identifier: { index },
		type: 'multisig',
		account: { address },
		metadata
	});

	const createRosettaCosignatory = (index, address) => ({
		operation_identifier: { index },
		type: 'cosign',
		account: { address }
	});

	// endregion

	// region derive

	describe('derive', () => {
		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			public_key: {
				hex_bytes: 'E85D10BF47FFBCE2230F70CB43ED2DDE04FCF342524B383972F86EA1FF773C79',
				curve_type: 'edwards25519'
			}
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) => {
			assertRosettaErrorRaisedBasic('/construction/derive', createValidRequest(), expectedError, malformRequest);
		};

		it('fails when request is invalid', () => assertRosettaErrorRaised(errors.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when curve type is unsupported', () => assertRosettaErrorRaised(errors.UNSUPPORTED_CURVE, request => {
			request.public_key.curve_type = 'secp256k1';
		}));

		it('fails when public key is invalid', () => assertRosettaErrorRaised(errors.INVALID_PUBLIC_KEY, request => {
			request.public_key.hex_bytes += '0';
		}));

		it('returns valid response on success', async () => {
			// Arrange:
			const expectedResponse = new ConstructionDeriveResponse();
			expectedResponse.account_identifier = new AccountIdentifier('TCHEST3QRQS4JZGOO64TH7NFJ2A63YA7TM2K5EQ');

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/derive', createValidRequest(), expectedResponse);
		});
	});

	// endregion

	// region preprocess

	describe('preprocess', () => {
		const createValidTransferRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			operations: [
				createRosettaTransfer(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaTransfer(1, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100')
			]
		});

		const createValidMultisigRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			operations: [
				createRosettaMultisig(0, 'TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ', {
					minApprovalDelta: 1,
					minRemovalDelta: 2,
					addressAdditions: ['TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ']
				}),
				createRosettaCosignatory(1, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'),
				createRosettaCosignatory(2, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ')
			]
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) => {
			assertRosettaErrorRaisedBasic('/construction/preprocess', createValidTransferRequest(), expectedError, malformRequest);
		};

		it('fails when request is invalid', () => assertRosettaErrorRaised(errors.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('returns valid response on success (transfer)', async () => {
			// Arrange:
			const expectedResponse = new ConstructionPreprocessResponse();
			expectedResponse.options = {};
			expectedResponse.required_public_keys = [
				new AccountIdentifier('TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/preprocess', createValidTransferRequest(), expectedResponse);
		});

		it('returns valid response on success (multisig)', async () => {
			// Arrange:
			const expectedResponse = new ConstructionPreprocessResponse();
			expectedResponse.options = {};
			expectedResponse.required_public_keys = [
				new AccountIdentifier('TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ'),
				new AccountIdentifier('TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'),
				new AccountIdentifier('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/preprocess', createValidMultisigRequest(), expectedResponse);
		});
	});

	// endregion

	// region metadata

	describe('metadata', () => {
		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier()
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) => {
			assertRosettaErrorRaisedBasic('/construction/metadata', createValidRequest(), expectedError, malformRequest);
		};

		const stubFetchResult = (ok, jsonResult) => {
			sinon.stub(global, 'fetch').returns(Promise.resolve({
				ok,
				json: () => jsonResult
			}));
		};

		afterEach(() => {
			if (global.fetch.restore)
				global.fetch.restore();
		});

		it('fails when request is invalid', () => assertRosettaErrorRaised(errors.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (headers)', async () => {
			// Arrange:
			stubFetchResult(false, {
				communicationTimestamps: {
					sendTimestamp: 1000,
					receiveTimestamp: 1001
				}
			});

			// Act + Assert:
			await assertRosettaErrorRaised(errors.CONNECTION_ERROR, () => {});
		});

		it('fails when fetch fails (body)', async () => {
			// Arrange:
			stubFetchResult(true, Promise.reject(Error('fetch failed')));

			// Act + Assert:
			await assertRosettaErrorRaised(errors.CONNECTION_ERROR, () => {});
		});

		it('returns valid response on success', async () => {
			// Arrange:
			stubFetchResult(true, {
				communicationTimestamps: {
					sendTimestamp: 1000,
					receiveTimestamp: 1001
				}
			});

			const expectedResponse = new ConstructionMetadataResponse();
			expectedResponse.metadata = {
				networkTime: 1001
			};

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/metadata', createValidRequest(), expectedResponse);
		});
	});

	// endregion

	// address => public key (might be needed in later tests)
	// TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI => 527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9
	// TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI => ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6
	// TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ => 93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7
	// TCJEJJBKDI62U4ZMO4VI7YAUVJE4STVCOBDSHXQ => 8A18D72A3D90547C9A2503C8513BC2A54D52B96119F4A5DF6E65CDA813EE5D9F
});
