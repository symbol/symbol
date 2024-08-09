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
import blockRoutes from '../../../../src/plugins/rosetta/nem/blockRoutes.js';
import Block from '../../../../src/plugins/rosetta/openApi/model/Block.js';
import BlockIdentifier from '../../../../src/plugins/rosetta/openApi/model/BlockIdentifier.js';
import BlockResponse from '../../../../src/plugins/rosetta/openApi/model/BlockResponse.js';
import BlockTransactionResponse from '../../../../src/plugins/rosetta/openApi/model/BlockTransactionResponse.js';
import Transaction from '../../../../src/plugins/rosetta/openApi/model/Transaction.js';
import TransactionIdentifier from '../../../../src/plugins/rosetta/openApi/model/TransactionIdentifier.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import { RosettaOperationFactory } from '../utils/rosettaTestUtils.js';
import { NemFacade } from 'symbol-sdk/nem';

describe('NEM block routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(blockRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(blockRoutes, ...args);

	// region utils

	const TEST_BLOCK_HEIGHT = 1111;
	const TRANSACTION_HASH = 'A5DEA9FA1CB0C88EB9EC9156602C68902FB2F61E13342E552603DC55FF199D89';
	const RECIPIENT_ADDRESS = 'TCYGRS7EWWHSNFUIETQIMB233NE75NVX6MJFI3JO';
	const SIGNER_PUBLICKEY = '61127CD45073DFED58F472748900D0B90E2D8EC6E10EE4E41B03861D1D1A720D';
	const SIGNER_ADDRESS = 'TCATGBEBY4GVVDEBDJCCSCYNK3XWORFB3R4PPZZU';

	const { createRosettaNetworkIdentifier } = RosettaObjectFactory;
	const createTransferOperation = (...args) =>
		RosettaOperationFactory.setOperationStatus(RosettaOperationFactory.createTransferOperation(...args));

	// endregion

	// region stub helpers

	const stubFetchResult = FetchStubHelper.stubPost;
	FetchStubHelper.registerStubCleanup();

	const createTransactionJson = (height = TEST_BLOCK_HEIGHT) => {
		const facade = new NemFacade('testnet');
		const textEncoder = new TextEncoder();
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction_v2',
			recipientAddress: RECIPIENT_ADDRESS,
			signerPublicKey: SIGNER_PUBLICKEY,
			amount: 4_000000,
			fee: 1000,
			mosaics: [
				{
					mosaic: {
						mosaicId: { namespaceId: { name: textEncoder.encode('nem') }, name: textEncoder.encode('xem') },
						amount: 10_000000n
					}
				},
				{
					mosaic: {
						mosaicId: {
							namespaceId: { name: textEncoder.encode('magic') },
							name: textEncoder.encode('hat')
						},
						amount: 5_00n
					}
				}
			]
		});

		return {
			transaction: convertTransactionSdkJsonToRestJson(transaction.toJson()),
			meta: {
				hash: { data: TRANSACTION_HASH },
				height
			}
		};
	};

	const createMatchingRosettaTransaction = () => {
		const transferCurrencyProperties = ['magic.hat', 3];
		const feeCurrencyProperties = ['nem.xem', 6];
		return new Transaction(
			new TransactionIdentifier(TRANSACTION_HASH),
			[
				createTransferOperation(0, SIGNER_ADDRESS, '-40000000', ...feeCurrencyProperties),
				createTransferOperation(1, RECIPIENT_ADDRESS, '40000000', ...feeCurrencyProperties),
				createTransferOperation(2, SIGNER_ADDRESS, '-2000', ...transferCurrencyProperties),
				createTransferOperation(3, RECIPIENT_ADDRESS, '2000', ...transferCurrencyProperties),
				createTransferOperation(4, SIGNER_ADDRESS, '-1000', ...feeCurrencyProperties)
			]
		);
	};

	const stubFetchNamespaceResult = (name, id, divisibility) => {
		stubFetchResult(`namespace/definition/page?namespace=${id}&pageSize=100`, true, {
			data: [
				{
					mosaic: {
						id: { namespaceId: id, name },
						properties: [
							{ name: 'divisibility', value: divisibility }
						],
						levy: {}
					}
				}
			]
		});
	};

	// endregion

	// region block

	describe('block', () => {
		const NEMESIS_BLOCK_HASH = 'AD8189C97983ED1D65C22236AFCD617D70A52A97D915DB2F9A76DC200AE1DD45';
		const OTHER_BLOCK_HASH = '6E2A9B4BE42D9DA4B9EA93C30E5E305553208D1BEA501340B2054AD0D44B9D8B';

		const createValidRequest = (height = TEST_BLOCK_HEIGHT) => ({
			network_identifier: createRosettaNetworkIdentifier(),
			block_identifier: { index: height.toString(), hash: 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076' }
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/block', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (local/block/at)', async () => {
			// Arrange:
			stubFetchResult('local/block/at', false, {});

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		const createBlocksResponse = (blockHeight, blockHash, blockTimestamp) => ({
			hash: blockHash,
			txes: [{
				tx: { ...createTransactionJson(blockHeight).transaction },
				hash: TRANSACTION_HASH
			}],
			block: {
				height: blockHeight,
				timeStamp: blockTimestamp,
				prevBlockHash: { data: NEMESIS_BLOCK_HASH }
			}
		});

		const stubPostLocalBlockAt = (blockHeight, blockHash, blockTimestamp, ok) => {
			FetchStubHelper.stubPost('local/block/at', ok, createBlocksResponse(blockHeight, blockHash, blockTimestamp), {
				method: 'POST',
				body: JSON.stringify({ height: blockHeight }),
				headers: { 'Content-Type': 'application/json' }
			});
		};

		const createMatchingRosettaBlock = (blockIdentifier, parentBlockIdentifier, timestamp) => {
			const expectedResponse = new BlockResponse();
			expectedResponse.block = new Block();
			expectedResponse.block.block_identifier = blockIdentifier;
			expectedResponse.block.parent_block_identifier = parentBlockIdentifier;
			expectedResponse.block.timestamp = (timestamp * 1000) + 1427587585000;
			expectedResponse.block.transactions = [createMatchingRosettaTransaction()];
			return expectedResponse;
		};

		it('succeeds when all fetches succeed (nemesis)', async () => {
			// Arrange:
			stubPostLocalBlockAt(1, NEMESIS_BLOCK_HASH, 0, true);
			stubFetchNamespaceResult('xem', 'nem', 6);
			stubFetchNamespaceResult('hat', 'magic', 3);

			// - create expected response
			const expectedResponse = createMatchingRosettaBlock(
				new BlockIdentifier(1, NEMESIS_BLOCK_HASH),
				new BlockIdentifier(1, NEMESIS_BLOCK_HASH),
				0
			);

			// Act + Assert:
			await assertRosettaSuccessBasic('/block', createValidRequest(1), expectedResponse);
		});

		it('succeeds when all fetches succeed (other block)', async () => {
			// Arrange:
			stubPostLocalBlockAt(1, NEMESIS_BLOCK_HASH, 0, true);
			stubPostLocalBlockAt(2, OTHER_BLOCK_HASH, 20, true);
			stubFetchNamespaceResult('xem', 'nem', 6);
			stubFetchNamespaceResult('hat', 'magic', 3);

			// - create expected response
			const expectedResponse = createMatchingRosettaBlock(
				new BlockIdentifier(2, OTHER_BLOCK_HASH),
				new BlockIdentifier(1, NEMESIS_BLOCK_HASH),
				20
			);

			// Act + Assert:
			await assertRosettaSuccessBasic('/block', createValidRequest(2), expectedResponse);
		});
	});

	// endregion

	// region block/transaction

	describe('block/transaction', () => {
		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			block_identifier: { index: '1111', hash: 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076' },
			transaction_identifier: { hash: TRANSACTION_HASH }
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/block/transaction', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (transactions/confirmed)', async () => {
			// Arrange:
			stubFetchResult(`transaction/get?hash=${TRANSACTION_HASH}`, false, {});

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when transaction is not part of block', async () => {
			// Arrange:
			stubFetchResult(`transaction/get?hash=${TRANSACTION_HASH}`, true, createTransactionJson());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
				request.block_identifier.index = '2222';
			});
		});

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			stubFetchResult(`transaction/get?hash=${TRANSACTION_HASH}`, true, createTransactionJson());
			stubFetchNamespaceResult('xem', 'nem', 6);
			stubFetchNamespaceResult('hat', 'magic', 3);

			// - create expected response
			const transaction = createMatchingRosettaTransaction();
			const expectedResponse = new BlockTransactionResponse(transaction);

			// Act + Assert:
			await assertRosettaSuccessBasic('/block/transaction', createValidRequest(), expectedResponse);
		});
	});

	// endregion
});
