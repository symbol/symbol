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
	assertRosettaSuccessBasicWithRoutes,
	createRosettaAggregateSignerKeyPair
} from './utils/rosettaTestUtils.js';
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
import constructionRoutes from '../../../../src/plugins/rosetta/symbol/constructionRoutes.js';
import sinon from 'sinon';
import { utils } from 'symbol-sdk';
import { models } from 'symbol-sdk/symbol';

describe('Symbol rosetta construction routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(constructionRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(constructionRoutes, ...args);

	// region type factories

	const { createRosettaNetworkIdentifier, createRosettaPublicKey } = RosettaObjectFactory;

	const createRosettaCurrency = () => ({
		symbol: 'symbol.xym',
		decimals: 6,
		metadata: { id: '1ABBCCDDAABBCCDD' } // resolved mosaic id
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
		const verifier = new PayloadResultVerifier();
		verifier.addTransfer(
			'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
			'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
			100n
		);

		verifier.buildAggregate(
			'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
			1001n + (60n * 60n * 1000n)
		);

		const operations = [
			createRosettaTransfer(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100'),
			createRosettaTransfer(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100')
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

	const createMultipleTransferTestCase = () => {
		const verifier = new PayloadResultVerifier();
		verifier.addTransfer(
			'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
			'TBOCOYGUXTPB6OQ4AOQXXTOW2ITP54AQ57EQRWY',
			87312
		);
		verifier.addTransfer(
			'086EA0653C38BBF4A3DD2C556C138DCDA6B5906638CF9D33E9A8B375A43F73A1',
			'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI',
			0
		);
		verifier.addTransfer(
			'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
			'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
			100n
		);
		verifier.addTransfer(
			'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
			'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI',
			50n
		);
		verifier.addTransfer(
			'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
			'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
			33n
		);

		verifier.buildAggregate(
			createRosettaAggregateSignerKeyPair().publicKey.toString(),
			1001n + (60n * 60n * 1000n),
			2
		);

		verifier.addCosignature('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7');
		verifier.addCosignature('ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6');

		verifier.setAggregateFeePayerSignature();

		return {
			verifier,
			operations: [
				createRosettaTransfer(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaTransfer(1, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100'),
				createRosettaTransfer(2, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '50'),
				createRosettaTransfer(3, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-50'),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '33'),
				createRosettaTransfer(5, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-33')
			],
			parsedOperations: [
				createRosettaTransfer(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-87312'), // transfer to fee payer
				createRosettaTransfer(1, 'TBOCOYGUXTPB6OQ4AOQXXTOW2ITP54AQ57EQRWY', '87312'),

				createRosettaTransfer(2, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100'),
				createRosettaTransfer(3, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-50'),
				createRosettaTransfer(5, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '50'),
				createRosettaTransfer(6, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-33'),
				createRosettaTransfer(7, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '33'),

				createRosettaCosignatory(8, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'),
				createRosettaCosignatory(9, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI')
			]
		};
	};

	const createMultipleTransferExplicitCosignerTestCase = () => {
		const verifier = new PayloadResultVerifier();
		verifier.addTransfer(
			'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
			'TBOCOYGUXTPB6OQ4AOQXXTOW2ITP54AQ57EQRWY',
			97920n
		);
		verifier.addTransfer(
			'086EA0653C38BBF4A3DD2C556C138DCDA6B5906638CF9D33E9A8B375A43F73A1',
			'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI',
			0
		);
		verifier.addTransfer(
			'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
			'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
			100n
		);
		verifier.addTransfer(
			'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
			'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI',
			50n
		);
		verifier.addTransfer(
			'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
			'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
			33n
		);

		verifier.buildAggregate(
			createRosettaAggregateSignerKeyPair().publicKey.toString(),
			1001n + (60n * 60n * 1000n),
			3
		);

		verifier.addCosignature('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623');
		verifier.addCosignature('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7');
		verifier.addCosignature('ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6');

		verifier.setAggregateFeePayerSignature();

		return {
			verifier,
			operations: [
				createRosettaTransfer(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaTransfer(1, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100'),
				createRosettaTransfer(2, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '50'),
				createRosettaTransfer(3, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-50'),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '33'),
				createRosettaTransfer(5, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-33'),
				createRosettaCosignatory(6, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ'),
				createRosettaCosignatory(7, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'), // redundant
				createRosettaCosignatory(8, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ') // redundant
			],
			parsedOperations: [
				createRosettaTransfer(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-97920'), // transfer to fee payer
				createRosettaTransfer(1, 'TBOCOYGUXTPB6OQ4AOQXXTOW2ITP54AQ57EQRWY', '97920'),

				createRosettaTransfer(2, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100'),
				createRosettaTransfer(3, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-50'),
				createRosettaTransfer(5, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '50'),
				createRosettaTransfer(6, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-33'),
				createRosettaTransfer(7, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '33'),

				createRosettaCosignatory(8, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ'),
				createRosettaCosignatory(9, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'),
				createRosettaCosignatory(10, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI')
			]
		};
	};

	const createSingleValidMultisigModificationTestCase = propertyName => {
		const metadata = {
			minApprovalDelta: 1,
			minRemovalDelta: 2,
			[propertyName]: ['TCIO5J4WTXCVC76XZPBWHHNAD2NU52U2MOOVN4Q', 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ']
		};

		const verifier = new PayloadResultVerifier();
		verifier.addTransfer(
			'3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623',
			'TBOCOYGUXTPB6OQ4AOQXXTOW2ITP54AQ57EQRWY',
			57936n
		);
		verifier.addTransfer(
			'086EA0653C38BBF4A3DD2C556C138DCDA6B5906638CF9D33E9A8B375A43F73A1',
			'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
			0
		);

		verifier.addMultisigModification('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623', { ...metadata });

		verifier.buildAggregate(
			createRosettaAggregateSignerKeyPair().publicKey.toString(),
			1001n + (60n * 60n * 1000n),
			1
		);

		verifier.addCosignature('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7');

		verifier.setAggregateFeePayerSignature();

		return {
			verifier,
			operations: [
				createRosettaMultisig(0, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', { ...metadata }),
				createRosettaCosignatory(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ')
			],
			parsedOperations: [
				createRosettaTransfer(0, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '-57936'), // transfer to fee payer
				createRosettaTransfer(1, 'TBOCOYGUXTPB6OQ4AOQXXTOW2ITP54AQ57EQRWY', '57936'),

				createRosettaMultisig(2, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', { ...metadata }),
				createRosettaCosignatory(3, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ')
			]
		};
	};

	// endregion

	// region derive

	describe('derive', () => {
		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			public_key: createRosettaPublicKey('E85D10BF47FFBCE2230F70CB43ED2DDE04FCF342524B383972F86EA1FF773C79')
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

		const createNodeTimeResult = () => ({
			communicationTimestamps: {
				sendTimestamp: 1000,
				receiveTimestamp: 1001
			}
		});

		const createNetworkFeesTransactionResult = () => ({
			averageFeeMultiplier: 102,
			medianFeeMultiplier: 100,
			highestFeeMultiplier: 543,
			lowestFeeMultiplier: 0,
			minFeeMultiplier: 10
		});

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (node/time)', async () => {
			// Arrange:
			stubFetchResult('node/time', false, createNodeTimeResult());
			stubFetchResult('network/fees/transaction', true, createNetworkFeesTransactionResult());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when fetch fails (network/fees/transaction)', async () => {
			// Arrange:
			stubFetchResult('node/time', true, createNodeTimeResult());
			stubFetchResult('network/fees/transaction', false, createNetworkFeesTransactionResult());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('returns valid response on success', async () => {
			// Arrange:
			stubFetchResult('node/time', true, createNodeTimeResult());
			stubFetchResult('network/fees/transaction', true, createNetworkFeesTransactionResult());

			// - create expected response
			const expectedResponse = new ConstructionMetadataResponse();
			expectedResponse.metadata = {
				networkTime: 1001,
				feeMultiplier: 102
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
				createRosettaTransfer(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaTransfer(1, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-100')
			],
			public_keys: [
				createRosettaPublicKey('527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9'),
				createRosettaPublicKey('ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6'),
				createRosettaPublicKey('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7'),
				createRosettaPublicKey('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623')
			],
			metadata: {
				networkTime: 1001,
				feeMultiplier: 102
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
					createRosettaTransfer(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
					createRosettaTransfer(1, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', '-99')
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
				verifier.makeSigningPayload('TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/payloads', request, expectedResponse);
		};

		it('succeeds when transfer has matched amounts (debit first)', () =>
			assertSingleValidTransfer(createSingleTransferDebitFirstTestCase()));

		it('succeeds when transfer has matched amounts (credit first)', () =>
			assertSingleValidTransfer(createSingleTransferCreditFirstTestCase()));

		it('succeeds when multiple transfers', async () => {
			// Arrange:
			const { verifier, operations } = createMultipleTransferTestCase();
			const request = createValidRequest();
			request.operations = operations;

			// - create expected response
			const expectedResponse = new ConstructionPayloadsResponse();
			expectedResponse.unsigned_transaction = verifier.toHexString();

			expectedResponse.payloads = [
				verifier.makeCosigningPayload('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'),
				verifier.makeCosigningPayload('TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/payloads', request, expectedResponse);
		});

		it('succeeds when multiple transfers with explicit cosigners', async () => {
			// Arrange:
			const { verifier, operations } = createMultipleTransferExplicitCosignerTestCase();
			const request = createValidRequest();
			request.operations = operations;

			// - create expected response
			const expectedResponse = new ConstructionPayloadsResponse();
			expectedResponse.unsigned_transaction = verifier.toHexString();

			expectedResponse.payloads = [
				verifier.makeCosigningPayload('TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ'),
				verifier.makeCosigningPayload('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'),
				verifier.makeCosigningPayload('TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI')
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/payloads', request, expectedResponse);
		});

		const assertSingleValidMultisigModification = async propertyName => {
			// Arrange:
			const { verifier, operations } = createSingleValidMultisigModificationTestCase(propertyName);
			const request = createValidRequest();
			request.operations = operations;

			// - create expected response
			const expectedResponse = new ConstructionPayloadsResponse();
			expectedResponse.unsigned_transaction = verifier.toHexString();

			expectedResponse.payloads = [
				verifier.makeCosigningPayload('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ') // only cosigner but not multisig account
			];

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/payloads', request, expectedResponse);
		};

		it('succeeds when multisig modification has additions', () => assertSingleValidMultisigModification('addressAdditions'));

		it('succeeds when multisig modification has deletions', () => assertSingleValidMultisigModification('addressDeletions'));
	});

	// endregion

	// region combine

	describe('combine', () => {
		const createSigningPayload = (address, publicKey, signaturePattern) => ({
			signing_payload: '',
			account_identifier: { address },
			public_key: createRosettaPublicKey(publicKey),
			hex_bytes: signaturePattern.repeat(64),
			signature_type: 'ed25519'
		});

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			signatures: [
				createSigningPayload(
					'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI',
					'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
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
			verifier.aggregateTransaction.signature = new models.Signature('11'.repeat(64));

			// - create expected response
			const expectedResponse = new ConstructionCombineResponse();
			expectedResponse.signed_transaction = verifier.toHexString();

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/combine', request, expectedResponse);
		});

		it('succeeds when cosignatures are required', async () => {
			// Arrange:
			const { verifier } = createSingleTransferCreditFirstTestCase();
			verifier.addCosignature('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7');
			verifier.addCosignature('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623');

			const request = createValidRequest();
			request.signatures.push(createSigningPayload(
				'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
				'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
				'22'
			));
			request.signatures.push(createSigningPayload(
				'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
				'3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623',
				'33'
			));
			request.unsigned_transaction = verifier.toHexString();

			// - add expected signatures
			verifier.aggregateTransaction.cosignatures[0].signature = new models.Signature('22'.repeat(64));
			verifier.aggregateTransaction.cosignatures[1].signature = new models.Signature('33'.repeat(64));

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

		const stubCurrencyMosaicIdRequest = () => {
			FetchStubHelper.stubCatapultProxyCacheFill();
			FetchStubHelper.stubPost('network/properties', true, { chain: { currencyMosaicId: '0x1ABBCCDDAABBCCDD' } });
			FetchStubHelper.stubMosaicResolution('1ABBCCDDAABBCCDD', 'symbol.xym', 6);
		};

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
			// Arrange:
			stubCurrencyMosaicIdRequest();

			// - clear the transaction data
			request.transaction = '';
		}));

		it('fails when mosaic is unsupported', () => assertRosettaErrorRaised(RosettaErrorFactory.NOT_SUPPORTED_ERROR, request => {
			// Arrange:
			stubCurrencyMosaicIdRequest();

			// - create an aggregate with an unsupported mosaic
			const verifier = new PayloadResultVerifier();
			verifier.addTransfer(
				'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
				'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
				100n
			);
			verifier.addTransfer(
				'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
				'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
				33n,
				'foo.bar'
			);

			verifier.buildAggregate(
				'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6',
				1001n + (60n * 60n * 1000n),
				1
			);

			request.transaction = verifier.toHexString();
		}));

		const assertRosettaSuccess = async (testCase, signed, expectedSigners = []) => {
			// Arrange:
			stubCurrencyMosaicIdRequest();

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

		it('succeeds when multiple transfers [unsigned]', () => assertRosettaSuccess(createMultipleTransferTestCase(), false));

		it('succeeds when multiple transfers with explicit cosigners [unsigned]', () =>
			assertRosettaSuccess(createMultipleTransferExplicitCosignerTestCase(), false));

		it('succeeds when multisig modification has additions [unsigned]', () =>
			assertRosettaSuccess(createSingleValidMultisigModificationTestCase('addressAdditions'), false));

		it('succeeds when multisig modification has deletions [unsigned]', () =>
			assertRosettaSuccess(createSingleValidMultisigModificationTestCase('addressDeletions'), false));

		it('succeeds when transfer has matched amounts [signed]', () =>
			assertRosettaSuccess(createSingleTransferCreditFirstTestCase(), true, ['TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI']));

		it('succeeds when multiple transfers [signed]', () => assertRosettaSuccess(createMultipleTransferTestCase(), true, [
			'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
			'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'
		]));

		it('succeeds when multiple transfers with explicit cosigners [signed]', () =>
			assertRosettaSuccess(createMultipleTransferExplicitCosignerTestCase(), true, [
				'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ',
				'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
				'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'
			]));

		it('succeeds when multisig modification has additions [signed]', () =>
			assertRosettaSuccess(createSingleValidMultisigModificationTestCase('addressAdditions'), true, [
				'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'
			]));

		it('succeeds when multisig modification has deletions [signed]', () =>
			assertRosettaSuccess(createSingleValidMultisigModificationTestCase('addressDeletions'), true, [
				'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'
			]));
	});

	// endregion

	// region hash

	const createBasicSignedTransaction = () => {
		const { verifier } = createSingleTransferCreditFirstTestCase();

		// add expected signatures
		verifier.aggregateTransaction.signature = new models.Signature('11'.repeat(64));
		return {
			aggregateTransaction: verifier.aggregateTransaction,
			transactionHash: verifier.facade.hashTransaction(verifier.aggregateTransaction)
		};
	};

	const createValidHashRequest = () => {
		const { aggregateTransaction } = createBasicSignedTransaction();
		const signedTransactionHex = utils.uint8ToHex(aggregateTransaction.serialize());

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
		const stubFetchResult = (signedTransactionHex, ok, jsonResult) => {
			if (!global.fetch.restore)
				sinon.stub(global, 'fetch');

			const fetchOptions = {
				method: 'PUT',
				headers: { 'Content-Type': 'application/json' },
				body: `{"payload":"${signedTransactionHex}"}`
			};
			global.fetch.withArgs('http://localhost:3456/transactions', fetchOptions).returns(Promise.resolve({
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
			const { aggregateTransaction } = createBasicSignedTransaction();
			const signedTransactionHex = utils.uint8ToHex(aggregateTransaction.serialize());

			stubFetchResult(signedTransactionHex, false, { message: 'SUCCESS' });

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('returns valid response on success', async () => {
			// Arrange:
			const { aggregateTransaction, transactionHash } = createBasicSignedTransaction();
			const signedTransactionHex = utils.uint8ToHex(aggregateTransaction.serialize());

			stubFetchResult(signedTransactionHex, true, { message: 'SUCCESS' });

			// - create expected response
			const expectedResponse = new TransactionIdentifierResponse();
			expectedResponse.transaction_identifier = new TransactionIdentifier(transactionHash.toString());

			// Act + Assert:
			await assertRosettaSuccessBasic('/construction/submit', createValidHashRequest(), expectedResponse);
		});
	});

	// endregion
});
