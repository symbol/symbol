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
	FetchStubHelper,
	RosettaObjectFactory,
	assertRosettaErrorRaisedBasicWithRoutes,
	assertRosettaSuccessBasicWithRoutes
} from './utils/rosettaTestUtils.js';
import { convertTransactionSdkJsonToRestJson } from '../../../../src/plugins/rosetta/nem/OperationParser.js';
import mempoolRoutes from '../../../../src/plugins/rosetta/nem/mempoolRoutes.js';
import MempoolResponse from '../../../../src/plugins/rosetta/openApi/model/MempoolResponse.js';
import MempoolTransactionResponse from '../../../../src/plugins/rosetta/openApi/model/MempoolTransactionResponse.js';
import Transaction from '../../../../src/plugins/rosetta/openApi/model/Transaction.js';
import TransactionIdentifier from '../../../../src/plugins/rosetta/openApi/model/TransactionIdentifier.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import { RosettaOperationFactory } from '../utils/rosettaTestUtils.js';
import { NemFacade } from 'symbol-sdk/nem';

describe('NEM rosetta mempool routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(mempoolRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(mempoolRoutes, ...args);

	// region type factories

	const TEST_SIGNATURES = [
		'126d0dd2a3c88875be474d05d84a79dc71644dfb2d5becb4c51423b8acafad6c8B70654E19E6EF355A290DF4D0575DE29D56CA8DBD799D8BCED27A8AC883325C',
		'c65df0b9cb47e1d3538dc40481fc613f37da4dee816f72fdf63061b2707f64832595F2F98A89757256F1783BA3518007467A7147A249C118E331554EC2A14C06',
		'7f6e1628c1624ff4abbbe8fdc445748dca9ddb9205062c3d26d4570ac646cde12A64DF983B061CE67C85707A2845B0EFEC1B98951E497CEFA671A4BF5ECA0166'
	];

	const TEST_SIGNATURES_UPPERCASE = [
		'126D0DD2A3C88875BE474D05D84A79DC71644DFB2D5BECB4C51423B8ACAFAD6C8B70654E19E6EF355A290DF4D0575DE29D56CA8DBD799D8BCED27A8AC883325C',
		'C65DF0B9CB47E1D3538DC40481FC613F37DA4DEE816F72FDF63061B2707F64832595F2F98A89757256F1783BA3518007467A7147A249C118E331554EC2A14C06',
		'7F6E1628C1624FF4ABBBE8FDC445748DCA9DDB9205062C3D26D4570AC646CDE12A64DF983B061CE67C85707A2845B0EFEC1B98951E497CEFA671A4BF5ECA0166'
	];

	const { createRosettaNetworkIdentifier, createRosettaPublicKey } = RosettaObjectFactory;
	const createTransferOperation = (...args) =>
		RosettaOperationFactory.setOperationStatus(RosettaOperationFactory.createTransferOperation(...args));

	const createExpectedFetchOptions = () => ({
		method: 'POST',
		headers: { 'Content-Type': 'application/json' },
		body: '{"challenge":{"data":"AAAAAAAAAA"},"entity":{"hashShortIds":[]}}'
	});

	const createUnconfirmedTransactionsResponse = () => {
		const textEncoder = new TextEncoder();
		const facade = new NemFacade('testnet');
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction_v2',
			signerPublicKey: '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E',
			fee: 50000,

			recipientAddress: 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV',
			amount: 2_000000,
			mosaics: [
				{
					mosaic: {
						mosaicId: { namespaceId: { name: textEncoder.encode('foo') }, name: textEncoder.encode('bar') },
						amount: 12345_000000
					}
				}
			]
		});

		return {
			signature: '',
			entity: {
				data: [
					{ signature: TEST_SIGNATURES[0] },
					{
						...convertTransactionSdkJsonToRestJson(transaction.toJson()),
						signature: TEST_SIGNATURES[1]
					},
					{ signature: TEST_SIGNATURES[2] }
				]
			}
		};
	};

	// endregion

	// region mempool

	describe('mempool', () => {
		const stubFetchResult = FetchStubHelper.stubPost;
		FetchStubHelper.registerStubCleanup();

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			public_key: createRosettaPublicKey('E85D10BF47FFBCE2230F70CB43ED2DDE04FCF342524B383972F86EA1FF773C79')
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/mempool', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (transactions/unconfirmed)', async () => {
			// Arrange:
			stubFetchResult('transactions/unconfirmed', false, createUnconfirmedTransactionsResponse(), createExpectedFetchOptions());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			stubFetchResult('transactions/unconfirmed', true, createUnconfirmedTransactionsResponse(), createExpectedFetchOptions());

			// - create expected response
			const expectedResponse = new MempoolResponse([
				new TransactionIdentifier(TEST_SIGNATURES_UPPERCASE[0]),
				new TransactionIdentifier(TEST_SIGNATURES_UPPERCASE[1]),
				new TransactionIdentifier(TEST_SIGNATURES_UPPERCASE[2])
			]);

			// Act + Assert:
			await assertRosettaSuccessBasic('/mempool', createValidRequest(), expectedResponse);
		});
	});

	// endregion

	// region mempool/transaction

	describe('mempool/transaction', () => {
		const stubFetchResult = FetchStubHelper.stubPost;
		FetchStubHelper.registerStubCleanup();

		const TRANSACTION_HASH = TEST_SIGNATURES_UPPERCASE[1];

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			transaction_identifier: { hash: TRANSACTION_HASH }
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/mempool/transaction', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (transactions/unconfirmed)', async () => {
			// Arrange:
			stubFetchResult('transactions/unconfirmed', false, createUnconfirmedTransactionsResponse(), createExpectedFetchOptions());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when transaction with hash does not exist', async () => {
			// Arrange:
			stubFetchResult('transactions/unconfirmed', true, createUnconfirmedTransactionsResponse(), createExpectedFetchOptions());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
				request.transaction_identifier.hash = `bb${TEST_SIGNATURES[1].substring(2)}`;
			});
		});

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			stubFetchResult('transactions/unconfirmed', true, createUnconfirmedTransactionsResponse(), createExpectedFetchOptions());
			FetchStubHelper.stubMosaicResolution('foo', 'bar', 3);

			// - create expected response
			const transferCurrencyProperties = ['foo:bar', 3];
			const feeCurrencyProperties = ['nem:xem', 6];
			const transaction = new Transaction(
				new TransactionIdentifier(TRANSACTION_HASH),
				[
					createTransferOperation(0, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-24690000000', ...transferCurrencyProperties),
					createTransferOperation(1, 'TALIC33LQMPC3DH73T5Y52SSVE2LRHSGRBGO4KIV', '24690000000', ...transferCurrencyProperties),
					createTransferOperation(2, 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW', '-50000', ...feeCurrencyProperties)
				]
			);
			const expectedResponse = new MempoolTransactionResponse(transaction);

			// Act + Assert:
			await assertRosettaSuccessBasic('/mempool/transaction', createValidRequest(), expectedResponse);
		});
	});

	// endregion
});
