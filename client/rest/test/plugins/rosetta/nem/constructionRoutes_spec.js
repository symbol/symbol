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
import ConstructionCombineResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionCombineResponse.js';
import ConstructionDeriveResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionDeriveResponse.js';
import ConstructionMetadataResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionMetadataResponse.js';
import ConstructionParseResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionParseResponse.js';
import ConstructionPayloadsResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionPayloadsResponse.js';
import ConstructionPreprocessResponse from '../../../../src/plugins/rosetta/openApi/model/ConstructionPreprocessResponse.js';
import TransactionIdentifier from '../../../../src/plugins/rosetta/openApi/model/TransactionIdentifier.js';
import TransactionIdentifierResponse from '../../../../src/plugins/rosetta/openApi/model/TransactionIdentifierResponse.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import { expect } from 'chai';
import sinon from 'sinon';
import { utils } from 'symbol-sdk';
import { TransactionFactory, models } from 'symbol-sdk/nem';

describe('NEM rosetta construction routes', () => {
	const FEE_UNIT = 50000n;

	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(constructionRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(constructionRoutes, ...args);

	// region type factories

	const { createRosettaNetworkIdentifier, createRosettaPublicKey } = RosettaObjectFactory;

	const createRosettaCurrency = () => ({
		symbol: 'nem:xem',
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
			10n * FEE_UNIT
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
			10n * FEE_UNIT
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
				createRosettaTransfer(0, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '100'),
				createRosettaTransfer(1, 'TALICE6KJ2SRSIJFVVFFH6ICUIYZ2ZZGNFUDJGRT', '-100')
			]
		});

		const createValidMultisigRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			operations: [
				createRosettaMultisig(0, 'TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH', {
					minApprovalDelta: 1,
					minRemovalDelta: 2,
					addressAdditions: ['TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', 'TALICE6KJ2SRSIJFVVFFH6ICUIYZ2ZZGNFUDJGRT'],
					addressDeletions: ['TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3']
				}),
				createRosettaCosignatory(1, 'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ'),
				createRosettaCosignatory(2, 'TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB')
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
				new AccountIdentifier('TALICE6KJ2SRSIJFVVFFH6ICUIYZ2ZZGNFUDJGRT')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/preprocess', createValidTransferRequest(), expectedResponse);
		});

		it('returns valid response on success (multisig)', async () => {
			// Arrange: create expected response (include address additions, address deletions and explicit cosignatories)
			const expectedResponse = new ConstructionPreprocessResponse();
			expectedResponse.options = {};
			expectedResponse.required_public_keys = [
				new AccountIdentifier('TDONALICE7O3L63AS3KNDCPT7ZA7HMQTFZGYUCAH'),
				new AccountIdentifier('TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV'),
				new AccountIdentifier('TALICE6KJ2SRSIJFVVFFH6ICUIYZ2ZZGNFUDJGRT'),
				new AccountIdentifier('TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3'),
				new AccountIdentifier('TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ'),
				new AccountIdentifier('TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB')
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

	// region combine

	describe('combine', () => {
		const createSigningPayload = (address, publicKey, signaturePattern) => ({
			signing_payload: '',
			account_identifier: { address },
			public_key: createRosettaPublicKey(publicKey),
			hex_bytes: signaturePattern.repeat(64),
			signature_type: 'ed25519_keccak'
		});

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			signatures: [
				createSigningPayload(
					'TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC',
					'E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778',
					'11'
				)
			],
			unsigned_transaction: ''
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/construction/combine', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when signature type is unsupported', () => assertRosettaErrorRaised(RosettaErrorFactory.UNSUPPORTED_CURVE, request => {
			request.signatures[0].signature_type = 'ecdsa';
		}));

		it('succeeds when no cosignatures are required', async () => {
			// Arrange:
			const { verifier } = createSingleTransferCreditFirstTestCase();

			const request = createValidRequest();
			request.unsigned_transaction = verifier.toHexString();

			// - add expected signatures
			verifier.transaction.signature = new models.Signature('11'.repeat(64));

			// - create expected response
			const expectedResponse = new ConstructionCombineResponse();
			expectedResponse.signed_transaction = verifier.toHexString();

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/combine', request, expectedResponse);
		});

		it('succeeds when cosignatures are required', async () => {
			// Arrange:
			const { verifier } = createMultisigSingleTransferCreditFirstTestCase();

			const request = createValidRequest();
			request.signatures.push(createSigningPayload(
				'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ',
				'5D2EC9959153F54E5225EBBC6A677AF37DCB8C3968558F366B2841F9DA9CA14F',
				'22'
			));
			request.signatures.push(createSigningPayload(
				'TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB',
				'4EEE569F2E0838489EAA0D55219D5738916D33B1379EF053920AD26548A2603E',
				'33'
			));
			request.signatures.push(createSigningPayload(
				'TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3',
				'88D0C34AEA2CB96E226379E71BA6264F4460C27D29F79E24248318397AA48380',
				'44'
			));
			request.unsigned_transaction = verifier.toHexString();

			// - add expected signatures
			verifier.transaction.signature = new models.Signature('22'.repeat(64));
			verifier.transaction.cosignatures[0].cosignature.signature = new models.Signature('33'.repeat(64));
			verifier.transaction.cosignatures[1].cosignature.signature = new models.Signature('44'.repeat(64));

			// - create expected response
			const expectedResponse = new ConstructionCombineResponse();
			expectedResponse.signed_transaction = verifier.toHexString();

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/combine', request, expectedResponse);
		});
	});

	// endregion

	// region parse

	describe('parse', () => {
		FetchStubHelper.registerStubCleanup();

		const createValidRequest = (signed = false) => {
			const { verifier } = createSingleTransferCreditFirstTestCase();
			const signedTransactionHex = verifier.toHexString();

			return {
				network_identifier: createRosettaNetworkIdentifier(),
				signed,
				transaction: signedTransactionHex
			};
		};

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/construction/parse', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when transaction is unparseable', () => assertRosettaErrorRaised(RosettaErrorFactory.INTERNAL_SERVER_ERROR, request => {
			// Arrange: clear the transaction data
			request.transaction = '';
		}));

		it('fails when mosaic is unsupported', () => assertRosettaErrorRaised(RosettaErrorFactory.NOT_SUPPORTED_ERROR, request => {
			// Arrange: create a transfer with an unsupported mosaic
			const verifier = new PayloadResultVerifier(1001);
			verifier.setTransferWithArbitraryMosaic(
				'E5F290755F021258ACE3CB29452BF38B322D76F62CAF6E9D2A89B48ABF7DD778',
				'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW'
			);

			request.transaction = verifier.toHexString();
		}));

		const assertRosettaSuccess = async (testCase, signed, expectedSigners = []) => {
			// Arrange:
			const { verifier, parsedOperations } = testCase;
			const request = createValidRequest(signed);
			request.transaction = verifier.toHexString();

			// - create expected response
			const expectedResponse = new ConstructionParseResponse();
			expectedResponse.operations = parsedOperations;
			expectedResponse.account_identifier_signers = expectedSigners.map(address => new AccountIdentifier(address));
			expectedResponse.signers = [];

			// Act + Assert: `parsedOperations` is a simple JS object, but `parse` builds up typed rosetta OpenAPI objects
			//               they cannot be compared directly, only indirectly via JSON
			await assertRosettaSuccessBasic('/construction/parse', request, expectedResponse, { roundtripJson: true });
		};

		it('succeeds when transfer has matched amounts [unsigned]', () =>
			assertRosettaSuccess(createSingleTransferCreditFirstTestCase(), false));

		it('succeeds when multisig modification [unsigned]', () =>
			assertRosettaSuccess(createSingleValidMultisigModificationTestCase(), false));

		it('succeeds when transfer in multisig [unsigned]', () =>
			assertRosettaSuccess(createMultisigSingleTransferCreditFirstTestCase(), false));

		it('succeeds when multisig modification in multisig [unsigned]', () =>
			assertRosettaSuccess(createMultisigSingleValidMultisigModificationTestCase(), false));

		it('succeeds when transfer has matched amounts [signed]', () =>
			assertRosettaSuccess(createSingleTransferCreditFirstTestCase(), true, ['TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC']));

		it('succeeds when multisig modification [signed]', () =>
			assertRosettaSuccess(createSingleValidMultisigModificationTestCase(), true, ['TBALNEMNEMKIMWLF65HTUWMQVX5G55EBBIWS4WQC']));

		it('succeeds when transfer in multisig [signed]', () =>
			assertRosettaSuccess(createMultisigSingleTransferCreditFirstTestCase(), true, [
				'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ',
				'TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB',
				'TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3'
			]));

		it('succeeds when multisig modification in multisig [signed]', () =>
			assertRosettaSuccess(createMultisigSingleValidMultisigModificationTestCase(), true, [
				'TBGJAGUAQY47BULYL4GRYBJLOI6XKXPJUXU25JRJ',
				'TAOPATMADWFEPME6GHOJL477SI7D3UT6NFJN4LGB'
			]));
	});

	// endregion

	// region hash

	const createBasicSignedTransaction = () => {
		const { verifier } = createSingleTransferCreditFirstTestCase();

		// add expected signatures
		verifier.transaction.signature = new models.Signature('11'.repeat(64));
		return {
			transaction: verifier.transaction,
			transactionHash: verifier.facade.hashTransaction(verifier.transaction)
		};
	};

	const createValidHashRequest = () => {
		const { transaction } = createBasicSignedTransaction();
		const signedTransactionHex = utils.uint8ToHex(transaction.serialize());

		return {
			network_identifier: createRosettaNetworkIdentifier(),
			signed_transaction: signedTransactionHex
		};
	};

	describe('hash', () => {
		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/construction/hash', createValidHashRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('succeeds when transaction is valid', async () => {
			// Arrange:
			const { transactionHash } = createBasicSignedTransaction();

			// - create expected response
			const expectedResponse = new TransactionIdentifierResponse();
			expectedResponse.transaction_identifier = new TransactionIdentifier(transactionHash.toString());

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/hash', createValidHashRequest(), expectedResponse);
		});
	});

	// endregion

	// region submit

	describe('submit', () => {
		const stubFetchResult = (transaction, ok, jsonResult) => {
			if (!global.fetch.restore)
				sinon.stub(global, 'fetch');

			const signedTransactionHex = utils.uint8ToHex(TransactionFactory.toNonVerifiableTransaction(transaction).serialize());
			const signatureHex = transaction.signature.toString();
			const fetchOptions = {
				method: 'POST',
				headers: { 'Content-Type': 'application/json' },
				body: `{"data":"${signedTransactionHex}","signature":"${signatureHex}"}`
			};
			global.fetch.withArgs('http://localhost:3456/transaction/announce', fetchOptions).returns(Promise.resolve({
				ok,
				json: () => jsonResult
			}));
		};

		FetchStubHelper.registerStubCleanup();

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/construction/submit', createValidHashRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails', async () => {
			// Arrange:
			const { transaction } = createBasicSignedTransaction();

			stubFetchResult(transaction, false, { message: 'SUCCESS' });

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when non-success message is returned', async () => {
			// Arrange:
			const { transaction } = createBasicSignedTransaction();

			stubFetchResult(transaction, true, { message: 'OTHER' });

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.INTERNAL_SERVER_ERROR, () => {});
		});

		it('returns valid response on success', async () => {
			// Arrange:
			const { transaction, transactionHash } = createBasicSignedTransaction();

			stubFetchResult(transaction, true, { message: 'SUCCESS' });

			// - create expected response
			const expectedResponse = new TransactionIdentifierResponse();
			expectedResponse.transaction_identifier = new TransactionIdentifier(transactionHash.toString());

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/submit', createValidHashRequest(), expectedResponse);
			expect(global.fetch.callCount).to.equal(1);
		});

		const createBasicMultisigTransactionWithCosignatures = () => {
			const { verifier } = createMultisigSingleTransferCreditFirstTestCase();

			// hoist cosignatures out of transaction
			const cosignatures = verifier.transaction.cosignatures.map(cosignature => cosignature.cosignature);
			verifier.transaction.cosignatures = [];
			return {
				transaction: verifier.transaction,
				transactionHash: verifier.facade.hashTransaction(verifier.transaction),
				cosignatures
			};
		};

		const createValidMultisigSubmitRequest = () => {
			const { verifier } = createMultisigSingleTransferCreditFirstTestCase();
			const signedTransactionHex = utils.uint8ToHex(verifier.transaction.serialize());

			return {
				network_identifier: createRosettaNetworkIdentifier(),
				signed_transaction: signedTransactionHex
			};
		};

		it('fails when any non-success message is returned (multisig)', async () => {
			// Arrange:
			const { transaction, cosignatures } = createBasicMultisigTransactionWithCosignatures();

			stubFetchResult(transaction, true, { message: 'SUCCESS' });
			stubFetchResult(cosignatures[0], true, { message: 'SUCCESS' });
			stubFetchResult(cosignatures[1], true, { message: 'OTHER' });

			// Act + Assert:
			assertRosettaErrorRaisedBasic(
				'/construction/submit',
				createValidMultisigSubmitRequest(),
				RosettaErrorFactory.INTERNAL_SERVER_ERROR
			);
		});

		it('returns valid response on success (multisig)', async () => {
			// Arrange:
			const { transaction, transactionHash, cosignatures } = createBasicMultisigTransactionWithCosignatures();

			stubFetchResult(transaction, true, { message: 'SUCCESS' });
			stubFetchResult(cosignatures[0], true, { message: 'SUCCESS' });
			stubFetchResult(cosignatures[1], true, { message: 'SUCCESS' });

			// - create expected response
			const expectedResponse = new TransactionIdentifierResponse();
			expectedResponse.transaction_identifier = new TransactionIdentifier(transactionHash.toString());

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/submit', createValidMultisigSubmitRequest(), expectedResponse);
			expect(global.fetch.callCount).to.equal(3);
		});
	});

	// endregion
});
