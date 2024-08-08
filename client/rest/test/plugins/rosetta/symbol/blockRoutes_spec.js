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
import Block from '../../../../src/plugins/rosetta/openApi/model/Block.js';
import BlockIdentifier from '../../../../src/plugins/rosetta/openApi/model/BlockIdentifier.js';
import BlockResponse from '../../../../src/plugins/rosetta/openApi/model/BlockResponse.js';
import BlockTransactionResponse from '../../../../src/plugins/rosetta/openApi/model/BlockTransactionResponse.js';
import Transaction from '../../../../src/plugins/rosetta/openApi/model/Transaction.js';
import TransactionIdentifier from '../../../../src/plugins/rosetta/openApi/model/TransactionIdentifier.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import { convertTransactionSdkJsonToRestJson } from '../../../../src/plugins/rosetta/symbol/OperationParser.js';
import blockRoutes from '../../../../src/plugins/rosetta/symbol/blockRoutes.js';
import { RosettaOperationFactory } from '../utils/rosettaTestUtils.js';
import { SymbolFacade, generateMosaicAliasId } from 'symbol-sdk/symbol';

describe('Symbol rosetta block routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(blockRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(blockRoutes, ...args);

	// region utils

	const TEST_BLOCK_HEIGHT = 1111;
	const TRANSACTION_HASH = 'C65DF0B9CB47E1D3538DC40481FC613F37DA4DEE816F72FDF63061B2707F6483';

	const { createRosettaNetworkIdentifier } = RosettaObjectFactory;
	const createTransferOperation = (...args) =>
		RosettaOperationFactory.setOperationStatus(RosettaOperationFactory.createTransferOperation(...args));

	const createTransactionJson = (height = TEST_BLOCK_HEIGHT) => {
		const facade = new SymbolFacade('testnet');
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction_v1',
			recipientAddress: 'TGJDIVTYSCV433YAAAAAAAAAAAAAAAAAAAAAAAA',
			signerPublicKey: '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9',
			mosaics: [
				{ mosaicId: generateMosaicAliasId('foo.bar'), amount: 9876 }
			],
			fee: 1000
		});

		return {
			transaction: convertTransactionSdkJsonToRestJson(transaction.toJson()),
			meta: {
				hash: TRANSACTION_HASH,
				height: height.toString(),
				index: 1
			}
		};
	};

	const createMatchingRosettaTransaction = () => {
		const transferCurrencyProperties = ['foo.bar', 3, '1122334455667788'];
		const feeCurrencyProperties = ['symbol.xym', 6, '1ABBCCDDAABBCCDD'];
		return new Transaction(
			new TransactionIdentifier(TRANSACTION_HASH),
			[
				createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-9876', ...transferCurrencyProperties),
				createTransferOperation(1, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '9876', ...transferCurrencyProperties),
				createTransferOperation(2, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-1000', ...feeCurrencyProperties)
			]
		);
	};

	const createStatmentWithReceipts = receipts => ({ statement: { receipts } });

	const createStatementsJson = () => ({
		data: [
			createStatmentWithReceipts([
				{
					type: 12626,
					targetAddress: '982390440A195B82D4A06810038B1400CE7CDA7AB3F48F99',
					mosaicId: '1ABBCCDDAABBCCDD',
					amount: '1234'
				},
				{
					type: 8515,
					targetAddress: '982390440A195B82D4A06810038B1400CE7CDA7AB3F48F99',
					mosaicId: '1122334455667788',
					amount: '4321'
				}
			]),
			createStatmentWithReceipts([
				{
					type: 4942,
					senderAddress: '985F73D67009A33962EFE43BD21288AFF7428B43FE942B5E',
					recipientAddress: '982390440A195B82D4A06810038B1400CE7CDA7AB3F48F99',
					mosaicId: '1122334455667788',
					amount: '888'
				}
			])
		]
	});

	const createMatchingReceiptsRosettaTransaction = transactionIdentifier => {
		const transferCurrencyProperties = ['foo.bar', 3, '1122334455667788'];
		const feeCurrencyProperties = ['symbol.xym', 6, '1ABBCCDDAABBCCDD'];
		return new Transaction(
			new TransactionIdentifier(transactionIdentifier),
			[
				createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-1234', ...feeCurrencyProperties),
				createTransferOperation(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '4321', ...transferCurrencyProperties),
				createTransferOperation(2, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '-888', ...transferCurrencyProperties),
				createTransferOperation(3, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '888', ...transferCurrencyProperties)
			]
		);
	};

	// endregion

	// region stub helpers

	const stubFetchResult = FetchStubHelper.stubPost;
	FetchStubHelper.registerStubCleanup();

	const stubTransactionResolutions = (height = TEST_BLOCK_HEIGHT) => {
		const makeResolutionStatement = (unresolved, resolutionEntries) => ({ statement: { unresolved, resolutionEntries } });
		const makeResolutionEntry = (resolved, primaryId, secondaryId) => ({ resolved, source: { primaryId, secondaryId } });

		// resolve unresolved address
		stubFetchResult(`statements/resolutions/address?height=${height}`, true, {
			data: [
				makeResolutionStatement('999234567890ABCDEF000000000000000000000000000000', [
					makeResolutionEntry('985F73D67009A33962EFE43BD21288AFF7428B43FE942B5E', 0, 0)
				])
			]
		});

		// resolve unresolved mosaic id
		stubFetchResult(`statements/resolutions/mosaic?height=${height}`, true, {
			data: [
				makeResolutionStatement('EC673E105521B12F', [
					makeResolutionEntry('1122334455667788', 0, 0)
				])
			]
		});
		FetchStubHelper.stubMosaicResolution('1122334455667788', 'foo.bar', 3);

		// resolve 'currencyMosaicId'
		FetchStubHelper.stubCatapultProxyCacheFill();
		stubFetchResult('network/properties', true, {
			network: { epochAdjustment: '112233s' },
			chain: { currencyMosaicId: '0x1ABBCCDDAABBCCDD' }
		});
		FetchStubHelper.stubMosaicResolution('1ABBCCDDAABBCCDD', 'symbol.xym', 6);
	};

	// endregion

	// region block

	describe('block', () => {
		const createValidRequest = (height = TEST_BLOCK_HEIGHT) => ({
			network_identifier: createRosettaNetworkIdentifier(),
			block_identifier: { index: height.toString(), hash: 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076' }
		});

		const createConfirmedTransactionsResponse = (height = TEST_BLOCK_HEIGHT) => ({
			data: [createTransactionJson(height)]
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/block', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (transactions/confirmed)', async () => {
			// Arrange:
			stubFetchResult(
				'transactions/confirmed?height=1111&embedded=true&pageNumber=1&pageSize=100',
				false,
				createConfirmedTransactionsResponse(1111)
			);

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		const stubBlockResolutions = height => {
			// setup block
			stubTransactionResolutions(height);
			stubFetchResult(`blocks/${height}`, true, {
				block: {
					height: height.toString(),
					previousBlockHash: '70C268A7F5B40F4E183BCE39E91B9A809D5612AB3BC1007AF7A15F477D421396',
					timestamp: '84055841'
				},
				meta: { hash: 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076' }
			});

			// setup block transactions
			stubFetchResult(
				`transactions/confirmed?height=${height}&embedded=true&pageNumber=1&pageSize=100`,
				true,
				createConfirmedTransactionsResponse(height)
			);

			// setup block statements
			stubFetchResult(`statements/transaction?height=${height}&pageNumber=1&pageSize=100`, true, createStatementsJson());
		};

		const createMatchingRosettaBlock = (blockIdentifier, parentBlockIdentifier) => {
			const expectedResponse = new BlockResponse();
			expectedResponse.block = new Block();
			expectedResponse.block.block_identifier = blockIdentifier;
			expectedResponse.block.parent_block_identifier = parentBlockIdentifier;
			expectedResponse.block.timestamp = (112233 * 1000) + 84055841;

			expectedResponse.block.transactions = [
				createMatchingRosettaTransaction(),
				createMatchingReceiptsRosettaTransaction(blockIdentifier.hash)
			];

			return expectedResponse;
		};

		it('succeeds when all fetches succeed (nemesis)', async () => {
			// Arrange:
			stubBlockResolutions(1);

			// - create expected response
			const expectedResponse = createMatchingRosettaBlock(
				new BlockIdentifier(1, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076'),
				new BlockIdentifier(1, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076')
			);

			// Act + Assert:
			await assertRosettaSuccessBasic('/block', createValidRequest(1), expectedResponse);
		});

		it('succeeds when all fetches succeed (other)', async () => {
			// Arrange:
			stubBlockResolutions(1111);

			// - create expected response
			const expectedResponse = createMatchingRosettaBlock(
				new BlockIdentifier(1111, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076'),
				new BlockIdentifier(1110, '70C268A7F5B40F4E183BCE39E91B9A809D5612AB3BC1007AF7A15F477D421396')
			);

			// Act + Assert:
			await assertRosettaSuccessBasic('/block', createValidRequest(1111), expectedResponse);
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
			stubFetchResult(`transactions/confirmed/${TRANSACTION_HASH}`, false, createTransactionJson());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('fails when transaction is not part of block', async () => {
			// Arrange:
			stubFetchResult(`transactions/confirmed/${TRANSACTION_HASH}`, true, createTransactionJson());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
				request.block_identifier.index = '2222';
			});
		});

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			stubFetchResult(`transactions/confirmed/${TRANSACTION_HASH}`, true, createTransactionJson());
			stubTransactionResolutions();

			// - create expected response
			const transaction = createMatchingRosettaTransaction();
			const expectedResponse = new BlockTransactionResponse(transaction);

			// Act + Assert:
			await assertRosettaSuccessBasic('/block/transaction', createValidRequest(), expectedResponse);
		});
	});

	// endregion
});
