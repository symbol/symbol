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
import MempoolResponse from '../../../../src/plugins/rosetta/openApi/model/MempoolResponse.js';
import MempoolTransactionResponse from '../../../../src/plugins/rosetta/openApi/model/MempoolTransactionResponse.js';
import Transaction from '../../../../src/plugins/rosetta/openApi/model/Transaction.js';
import TransactionIdentifier from '../../../../src/plugins/rosetta/openApi/model/TransactionIdentifier.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import { convertTransactionSdkJsonToRestJson } from '../../../../src/plugins/rosetta/symbol/OperationParser.js';
import mempoolRoutes from '../../../../src/plugins/rosetta/symbol/mempoolRoutes.js';
import { RosettaOperationFactory } from '../utils/rosettaTestUtils.js';
import { SymbolFacade, generateMosaicAliasId } from 'symbol-sdk/symbol';

describe('Symbol rosetta mempool routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(mempoolRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(mempoolRoutes, ...args);

	// region type factories

	const { createRosettaNetworkIdentifier, createRosettaPublicKey } = RosettaObjectFactory;
	const createTransferOperation = (...args) =>
		RosettaOperationFactory.setOperationStatus(RosettaOperationFactory.createTransferOperation(...args));

	// endregion

	// region mempool

	describe('mempool', () => {
		const stubFetchResult = FetchStubHelper.stubPost;
		FetchStubHelper.registerStubCleanup();

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			public_key: createRosettaPublicKey('E85D10BF47FFBCE2230F70CB43ED2DDE04FCF342524B383972F86EA1FF773C79')
		});

		const createUnconfirmedTransactionsResponse = () => ({
			data: [
				{ meta: { hash: '126D0DD2A3C88875BE474D05D84A79DC71644DFB2D5BECB4C51423B8ACAFAD6C' } },
				{ meta: { hash: 'C65DF0B9CB47E1D3538DC40481FC613F37DA4DEE816F72FDF63061B2707F6483' } },
				{ meta: { hash: '7F6E1628C1624FF4ABBBE8FDC445748DCA9DDB9205062C3D26D4570AC646CDE1' } }
			]
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/mempool', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (transactions/unconfirmed)', async () => {
			// Arrange:
			stubFetchResult('transactions/unconfirmed?pageNumber=1&pageSize=100', false, createUnconfirmedTransactionsResponse());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			stubFetchResult('transactions/unconfirmed?pageNumber=1&pageSize=100', true, createUnconfirmedTransactionsResponse());

			// - create expected response
			const expectedResponse = new MempoolResponse([
				new TransactionIdentifier('126D0DD2A3C88875BE474D05D84A79DC71644DFB2D5BECB4C51423B8ACAFAD6C'),
				new TransactionIdentifier('C65DF0B9CB47E1D3538DC40481FC613F37DA4DEE816F72FDF63061B2707F6483'),
				new TransactionIdentifier('7F6E1628C1624FF4ABBBE8FDC445748DCA9DDB9205062C3D26D4570AC646CDE1')
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

		const TRANSACTION_HASH = 'C65DF0B9CB47E1D3538DC40481FC613F37DA4DEE816F72FDF63061B2707F6483';

		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			transaction_identifier: { hash: TRANSACTION_HASH }
		});

		const createTransactionJson = () => {
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
				meta: { hash: TRANSACTION_HASH }
			};
		};

		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic('/mempool/transaction', createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (transactions/unconfirmed)', async () => {
			// Arrange:
			stubFetchResult(`transactions/unconfirmed/${TRANSACTION_HASH}`, false, createTransactionJson());

			// Act + Assert:
			await assertRosettaErrorRaised(RosettaErrorFactory.CONNECTION_ERROR, () => {});
		});

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			stubFetchResult(`transactions/unconfirmed/${TRANSACTION_HASH}`, true, createTransactionJson());

			// - resolve unresolved address
			stubFetchResult('namespaces/9234567890ABCDEF', true, {
				namespace: {
					alias: {
						address: '985F73D67009A33962EFE43BD21288AFF7428B43FE942B5E'
					}
				}
			});

			// - resolve unresolved mosaic id
			stubFetchResult('namespaces/EC673E105521B12F', true, { namespace: { alias: { mosaicId: '1122334455667788' } } });
			FetchStubHelper.stubMosaicResolution('1122334455667788', 'foo.bar', 3);

			// - resolve 'currencyMosaicId'
			FetchStubHelper.stubCatapultProxyCacheFill();
			stubFetchResult('network/properties', true, { chain: { currencyMosaicId: '0x1ABBCCDDAABBCCDD' } });
			FetchStubHelper.stubMosaicResolution('1ABBCCDDAABBCCDD', 'symbol.xym', 6);

			// - create expected response
			const transferCurrencyProperties = ['foo.bar', 3, '1122334455667788'];
			const feeCurrencyProperties = ['symbol.xym', 6, '1ABBCCDDAABBCCDD'];
			const transaction = new Transaction(
				new TransactionIdentifier(TRANSACTION_HASH),
				[
					createTransferOperation(0, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-9876', ...transferCurrencyProperties),
					createTransferOperation(1, 'TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ', '9876', ...transferCurrencyProperties),
					createTransferOperation(2, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '-1000', ...feeCurrencyProperties)
				]
			);
			const expectedResponse = new MempoolTransactionResponse(transaction);

			// Act + Assert:
			await assertRosettaSuccessBasic('/mempool/transaction', createValidRequest(), expectedResponse);
		});
	});

	// endregion
});
