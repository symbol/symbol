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

import PayloadResultVerifier from './utils/PayloadResultVerifier.js';
import {
	FetchStubHelper,
	RosettaObjectFactory,
	assertRosettaErrorRaisedBasicWithRoutes,
	assertRosettaSuccessBasicWithRoutes
} from './utils/rosettaTestUtils.js';
import constructionRoutes from '../../../../src/plugins/rosetta/nem/constructionRoutes.js';
import AccountIdentifier from '../../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import ConstructionDeriveResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionDeriveResponse.js';
import ConstructionMetadataResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionMetadataResponse.js';
import ConstructionPayloadsResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionPayloadsResponse.js';
import ConstructionPreprocessResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionPreprocessResponse.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';

describe('construction routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(constructionRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(constructionRoutes, ...args);

	// region type factories

	const { createRosettaNetworkIdentifier, createRosettaPublicKey } = RosettaObjectFactory;

	const createRosettaCurrency = () => ({
		symbol: 'nem.xem',
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
		metadata: {
			addressAdditions: [],
			addressDeletions: [],
			...metadata
		}
	});

	const createRosettaCosignatory = (index, address) => ({
		operation_identifier: { index },
		type: 'cosign',
		account: { address }
	});

	// endregion

	// region utils - TestCases

	const createSingleTransferDebitFirstTestCase = () => {
		const verifier = new PayloadResultVerifier(1001);
		verifier.setTransfer(
			'E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778',
			'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
			100000n,
			50000n * 10n
		);

		const operations = [
			createRosettaTransfer(0, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', '-100000'),
			createRosettaTransfer(1, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '100000')
		];
		return { verifier, operations, parsedOperations: operations };
	};

	const createSingleTransferCreditFirstTestCase = () => {
		const testCase = createSingleTransferDebitFirstTestCase();

		// make deep copy of operation_identifier to avoid modifying parsedOperations
		testCase.operations = testCase.operations.map((operation, i) => ({
			...operation,
			operation_identifier: { index: testCase.operations.length - 1 - i }
		}));
		testCase.operations.reverse();

		return testCase;
	};

	const createMultisigSingleTransferCreditFirstTestCase = () => {
		const verifier = new PayloadResultVerifier(1001);
		verifier.setTransfer(
			'E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778',
			'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW',
			100000n,
			50000n * 10n
		);

		verifier.makeMultisig('5D2EC9959153F54E5225EBBC6A677AF37DCB8C3968558F366B2841F9DA9CA14F', [
			'4EEE569F2E0838489EAA0D55219D5738916D33B1379EF053920AD26548A2603E',
			'88D0C34AEA2CB96E226379E71BA6264F4460C27D29F79E24248318397AA48380'
		]);

		const operations = [
			createRosettaTransfer(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '100000'),
			createRosettaTransfer(1, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', '-100000'),
			createRosettaCosignatory(2, 'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ'),
			createRosettaCosignatory(3, 'TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB'),
			createRosettaCosignatory(4, 'TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3')
		];
		return {
			verifier,
			operations,
			parsedOperations: [
				createRosettaTransfer(0, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', '-100000'),
				createRosettaTransfer(1, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '100000'),
				...operations.slice(2)
			]
		};
	};

	const createSingleValidMultisigModificationTestCase = () => {
		const verifier = new PayloadResultVerifier(1001);
		verifier.setMultisigModification('E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778', {
			minApprovalDelta: 1,
			modifications: [
				{
					modification: {
						modificationType: 'add_cosignatory',
						cosignatoryPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E'
					}
				},
				{
					modification: {
						modificationType: 'delete_cosignatory',
						cosignatoryPublicKey: '5D2EC9959153F54E5225EBBC6A677AF37DCB8C3968558F366B2841F9DA9CA14F'
					}
				},
				{
					modification: {
						modificationType: 'add_cosignatory',
						cosignatoryPublicKey: '4EEE569F2E0838489EAA0D55219D5738916D33B1379EF053920AD26548A2603E'
					}
				},
				{
					modification: {
						modificationType: 'delete_cosignatory',
						cosignatoryPublicKey: '88D0C34AEA2CB96E226379E71BA6264F4460C27D29F79E24248318397AA48380'
					}
				}
			]
		});

		const operations = [
			createRosettaMultisig(0, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', {
				minApprovalDelta: 1,
				addressAdditions: ['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', 'TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB'],
				addressDeletions: ['TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ', 'TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3']
			})
		];
		return { verifier, operations, parsedOperations: operations };
	};

	const createMultisigSingleValidMultisigModificationTestCase = () => {
		const verifier = new PayloadResultVerifier(1001);
		verifier.setMultisigModification('E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778', {
			minApprovalDelta: 1,
			modifications: [
				{
					modification: {
						modificationType: 'add_cosignatory',
						cosignatoryPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E'
					}
				},
				{
					modification: {
						modificationType: 'delete_cosignatory',
						cosignatoryPublicKey: '88D0C34AEA2CB96E226379E71BA6264F4460C27D29F79E24248318397AA48380'
					}
				}
			]
		});

		verifier.makeMultisig('5D2EC9959153F54E5225EBBC6A677AF37DCB8C3968558F366B2841F9DA9CA14F', [
			'4EEE569F2E0838489EAA0D55219D5738916D33B1379EF053920AD26548A2603E'
		]);

		const operations = [
			createRosettaMultisig(0, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', {
				minApprovalDelta: 1,
				addressAdditions: ['TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW'],
				addressDeletions: ['TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3']
			}),
			createRosettaCosignatory(1, 'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ'),
			createRosettaCosignatory(2, 'TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB')
		];
		return { verifier, operations, parsedOperations: operations };
	};

	// endregion

	// region derive

	describe('derive', () => {
		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			public_key: createRosettaPublicKey('DEE852011049D890E97DD4F1D4D7F76824C16854578DADA9918BF943FBF5CC13')
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/construction/derive', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when public key is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_PUBLIC_KEY, request => {
			request.public_key.hex_bytes += '0';
		}));

		it('returns valid response on success', async () => {
			// Arrange: create expected response
			const expectedResponse = new ConstructionDeriveResponse();
			expectedResponse.account_identifier = new AccountIdentifier('TCHESTYVD2P6P646AMY7WSNG73PCPZDUQPN4KCPY');

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

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/construction/preprocess', createValidTransferRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('returns valid response on success (transfer)', async () => {
			// Arrange: create expected response
			const expectedResponse = new ConstructionPreprocessResponse();
			expectedResponse.options = {};
			expectedResponse.required_public_keys = [
				new AccountIdentifier('TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/preprocess', createValidTransferRequest(), expectedResponse);
		});

		it('returns valid response on success (multisig)', async () => {
			// Arrange: create expected response
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
		const stubFetchResult = FetchStubHelper.stubPost;
		FetchStubHelper.registerStubCleanup();

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier()
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/construction/metadata', createValidRequest(), expectedError, malformRequest);

		const createTimeSyncNetworkTimeResult = () => ({
			sendTimeStamp: 1000100,
			receiveTimeStamp: 1001100
		});

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (time-sync/network-time)', async () => {
			// Arrange:
			stubFetchResult('time-sync/network-time', false, createTimeSyncNetworkTimeResult());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('returns valid response on success', async () => {
			// Arrange:
			stubFetchResult('time-sync/network-time', true, createTimeSyncNetworkTimeResult());

			// - create expected response
			const expectedResponse = new ConstructionMetadataResponse();
			expectedResponse.metadata = {
				networkTime: 1001
			};

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/metadata', createValidRequest(), expectedResponse);
		});
	});

	// endregion

	// region payloads

	describe('payloads', () => {
		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			operations: [
				createRosettaTransfer(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '100'),
				createRosettaTransfer(1, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', '-100')
			],
			public_keys: [
				createRosettaPublicKey('9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E'),
				createRosettaPublicKey('E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778'),
				createRosettaPublicKey('5D2EC9959153F54E5225EBBC6A677AF37DCB8C3968558F366B2841F9DA9CA14F'),
				createRosettaPublicKey('4EEE569F2E0838489EAA0D55219D5738916D33B1379EF053920AD26548A2603E'),
				createRosettaPublicKey('88D0C34AEA2CB96E226379E71BA6264F4460C27D29F79E24248318397AA48380')
			],
			metadata: {
				networkTime: 1001
			}
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/construction/payloads', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when any public key is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_PUBLIC_KEY, request => {
			request.public_keys[1].hex_bytes += '0';
		}));

		it('fails when any transfer has mismatched amounts', () => assertRosettaErrorRaised(
			RosettaErrorFactory.INVALID_REQUEST_DATA,
			request => {
				request.operations = [
					createRosettaTransfer(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '100'),
					createRosettaTransfer(1, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', '-99')
				];
			}
		));

		it('fails when there are multiple operations (transfer)', () => assertRosettaErrorRaised(
			RosettaErrorFactory.INVALID_REQUEST_DATA,
			request => {
				request.operations = [
					createRosettaTransfer(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '100'),
					createRosettaTransfer(1, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', '-100'),
					createRosettaTransfer(2, 'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH', '100'),
					createRosettaTransfer(3, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', '-100')
				];
			}
		));

		it('fails when there are multiple operations (multisig)', () => assertRosettaErrorRaised(
			RosettaErrorFactory.INVALID_REQUEST_DATA,
			request => {
				request.operations = [
					createRosettaTransfer(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '100'),
					createRosettaTransfer(1, 'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC', '-100'),
					createRosettaMultisig(0, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', {})
				];
			}
		));

		const assertSingleValidTransfer = async testCase => {
			// Arrange:
			const { verifier } = testCase;
			const request = createValidRequest();
			request.operations = testCase.operations;

			// - create expected response
			const expectedResponse = new ConstructionPayloadsResponse();
			expectedResponse.unsigned_transaction = verifier.toHexString();

			expectedResponse.payloads = [
				verifier.makeSigningPayload('TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/payloads', request, expectedResponse);
		};

		it('succeeds when transfer has matched amounts (debit first)', () =>
			assertSingleValidTransfer(createSingleTransferDebitFirstTestCase()));

		it('succeeds when transfer has matched amounts (credit first)', () =>
			assertSingleValidTransfer(createSingleTransferCreditFirstTestCase()));

		it('succeeds when multisig modification', async () => {
			// Arrange:
			const { verifier, operations } = createSingleValidMultisigModificationTestCase();
			const request = createValidRequest();
			request.operations = operations;

			// - create expected response
			const expectedResponse = new ConstructionPayloadsResponse();
			expectedResponse.unsigned_transaction = verifier.toHexString();

			expectedResponse.payloads = [
				verifier.makeSigningPayload('TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/payloads', request, expectedResponse);
		});

		it('succeeds when transfer in multisig', async () => {
			// Arrange:
			const { verifier, operations } = createMultisigSingleTransferCreditFirstTestCase();
			const request = createValidRequest();
			request.operations = operations;

			// - create expected response
			const expectedResponse = new ConstructionPayloadsResponse();
			expectedResponse.unsigned_transaction = verifier.toHexString();

			expectedResponse.payloads = [
				verifier.makeSigningPayload('TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ'),
				verifier.makeCosigningPayload('TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB'),
				verifier.makeCosigningPayload('TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/payloads', request, expectedResponse);
		});

		it('succeeds when multisig modification in multisig', async () => {
			// Arrange:
			const { verifier, operations } = createMultisigSingleValidMultisigModificationTestCase();
			const request = createValidRequest();
			request.operations = operations;

			// - create expected response
			const expectedResponse = new ConstructionPayloadsResponse();
			expectedResponse.unsigned_transaction = verifier.toHexString();

			expectedResponse.payloads = [
				verifier.makeSigningPayload('TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ'),
				verifier.makeCosigningPayload('TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/payloads', request, expectedResponse);
		});
	});

	// endregion
});
