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
import accountRoutes from '../../../../src/plugins/rosetta/nem/accountRoutes.js';
import AccountBalanceResponse from '../../../../src/plugins/rosetta/openApi/model/AccountBalanceResponse.js';
import AccountCoinsResponse from '../../../../src/plugins/rosetta/openApi/model/AccountCoinsResponse.js';
import Amount from '../../../../src/plugins/rosetta/openApi/model/Amount.js';
import BlockIdentifier from '../../../../src/plugins/rosetta/openApi/model/BlockIdentifier.js';
import Coin from '../../../../src/plugins/rosetta/openApi/model/Coin.js';
import CoinIdentifier from '../../../../src/plugins/rosetta/openApi/model/CoinIdentifier.js';
import { RosettaErrorFactory } from '../../../../src/plugins/rosetta/rosettaUtils.js';
import { RosettaOperationFactory } from '../utils/rosettaTestUtils.js';
import { NemFacade } from 'symbol-sdk/nem';

describe('NEM rosetta account routes', () => {
	const ACCOUNT_ADDRESS = 'TALICE5VF6J5FYMTCB7A3QG6OIRDRUXDWJGFVXNW';
	const ACCOUNT_PUBLIC_KEY = '9822CF9571A5551EC19720B87A567A20797B75EC4B6711387643FC352FEF704E';
	const OTHER_ACCOUNT_ADDRESS = 'TBMKRYST2J3GEZRWHS3MICWFIBSKVHH7F5FA6FH3';
	const OTHER_ACCOUNT_PUBLIC_KEY = '88D0C34AEA2CB96E226379E71BA6264F4460C27D29F79E24248318397AA48380';
	const TRANSACTION_HASH = 'c65df0b9cb47e1d3538dc40481fc613f37da4dee816f72fdf63061b2707f6483';

	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(accountRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(accountRoutes, ...args);

	// region utils

	const { createRosettaNetworkIdentifier } = RosettaObjectFactory;

	const createRosettaAmount = (amount, currencyName, currencyDecimals) => new Amount(
		amount,
		RosettaOperationFactory.createCurrency(currencyName, currencyDecimals)
	);

	const createRosettaCoin = (amount, currencyName, currencyDecimals) => new Coin(
		new CoinIdentifier(currencyName),
		createRosettaAmount(amount, currencyName, currencyDecimals)
	);

	const stubFetchResult = FetchStubHelper.stubPost;
	FetchStubHelper.registerStubCleanup();

	const stubAccountResolutions = () => {
		// setup chain
		stubFetchResult('chain/height', true, { height: 12345 });
		FetchStubHelper.stubLocalBlockAt({ height: 12345, hash: 'a4950f27a23b235d5ccd1dc7ff4b0bdc48977e353ea1cf1e3e5f70b9a6b79076' }, true);

		// setup account
		stubFetchResult(`account/mosaic/owned?address=${ACCOUNT_ADDRESS}`, true, {
			data: [
				{ mosaicId: { namespaceId: 'foo', name: 'bar' }, quantity: 123 },
				{ mosaicId: { namespaceId: 'cat', name: 'dog' }, quantity: 262 },
				{ mosaicId: { namespaceId: 'nem', name: 'xem' }, quantity: 112 }
			]
		});

		// resolve mosaics
		FetchStubHelper.stubMosaicResolution('foo', 'bar', 3);
		FetchStubHelper.stubMosaicResolution('cat', 'dog', 4);
		FetchStubHelper.stubMosaicResolution('new', 'coin', 5);
	};

	// endregion

	// region common tests

	const addAccountFailureTests = (route, options) => {
		const assertRosettaErrorRaised = (expectedError, malformRequest) =>
			assertRosettaErrorRaisedBasic(route, options.createValidRequest(), expectedError, malformRequest);

		it('fails when request is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			delete request.network_identifier.network;
		}));

		it('fails when fetch fails (chain/info)', async () => {
			stubFetchResult('chain/info', false, { height: '12345' });
		});

		if (!options.skipBlockIdentifierTest) {
			it('fails when block identifier is provided', () => assertRosettaErrorRaised(
				RosettaErrorFactory.NOT_SUPPORTED_ERROR,
				request => {
					request.block_identifier = { index: '1111', hash: 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076' };
				}
			));
		}

		it('fails when address is invalid', () => assertRosettaErrorRaised(RosettaErrorFactory.INVALID_REQUEST_DATA, request => {
			request.account_identifier.address = 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKX';
		}));

		it('fails when block changes during account query', () => assertRosettaErrorRaised(
			RosettaErrorFactory.SYNC_DURING_OPERATION,
			() => {
				stubFetchResult('chain/height', true, { height: 12345 });
				stubFetchResult('chain/height', true, { height: 12346 }, undefined, 'Second');
				stubFetchResult(`account/mosaic/owned?address=${ACCOUNT_ADDRESS}`, true, { account: { mosaics: [] } });
			}
		));
	};

	const addAccountSuccessTests = (route, options) => {
		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			stubAccountResolutions();

			// - create expected response
			const expectedResponse = new options.Response(
				new BlockIdentifier(12345, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076'),
				[
					options.createRosettaAmount('123', 'foo:bar', 3),
					options.createRosettaAmount('262', 'cat:dog', 4),
					options.createRosettaAmount('112', 'nem:xem', 6)
				]
			);

			// Act + Assert:
			await assertRosettaSuccessBasic(route, options.createValidRequest(), expectedResponse);
		});

		it('succeeds when all fetches succeed and applies currency filter', async () => {
			// Arrange:
			stubAccountResolutions();

			// - add currency filter
			const request = options.createValidRequest();
			request.currencies = [
				{ symbol: 'foo:baz', decimals: 3 }, // name mismatch
				{ symbol: 'cat:dog', decimals: 4 },
				{ symbol: 'nem:xem', decimals: 1 } // decimals mismatch
			];

			// - create expected response
			const expectedResponse = new options.Response(
				new BlockIdentifier(12345, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076'),
				[
					options.createRosettaAmount('262', 'cat:dog', 4)
				]
			);

			// Act + Assert:
			await assertRosettaSuccessBasic(route, request, expectedResponse);
		});
	};

	// endregion

	// region balance

	describe('balance', () => {
		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			account_identifier: { address: ACCOUNT_ADDRESS }
		});

		addAccountFailureTests('/account/balance', { createValidRequest });

		addAccountSuccessTests('/account/balance', {
			createValidRequest,
			createRosettaAmount,
			Response: AccountBalanceResponse
		});
	});

	// endregion

	// region coins

	describe('coins', () => {
		const createValidRequest = () => ({
			network_identifier: createRosettaNetworkIdentifier(),
			account_identifier: { address: ACCOUNT_ADDRESS },
			include_mempool: false
		});

		addAccountFailureTests('/account/coins', {
			createValidRequest,
			skipBlockIdentifierTest: true
		});

		addAccountSuccessTests('/account/coins', {
			createValidRequest,
			createRosettaAmount: createRosettaCoin,
			Response: AccountCoinsResponse
		});

		const createTransferTransaction = (mosaics, signerPublicKey, recipientAddress) => {
			const textEncoder = new TextEncoder();
			const facade = new NemFacade('testnet');
			return facade.transactionFactory.create({
				type: 'transfer_transaction_v2',
				signerPublicKey,
				fee: 50,

				recipientAddress,
				amount: 1_000000,
				mosaics: mosaics.map(mosaicDescriptor => ({
					mosaic: {
						mosaicId: {
							namespaceId: { name: textEncoder.encode(mosaicDescriptor.namespaceId) },
							name: textEncoder.encode(mosaicDescriptor.name)
						},
						amount: mosaicDescriptor.amount
					}
				}))
			});
		};

		const createTransactionJson = (mosaics, signerPublicKey, recipientAddress) => {
			const transaction = createTransferTransaction(mosaics, signerPublicKey, recipientAddress);
			return {
				transaction: convertTransactionSdkJsonToRestJson(transaction.toJson()),
				meta: { hash: { data: TRANSACTION_HASH } }
			};
		};

		const createAggregateTransactionJson = (mosaics, signerPublicKey, recipientAddress) => {
			const facade = new NemFacade('testnet');

			const transaction = createTransferTransaction(mosaics, signerPublicKey, recipientAddress);
			const multisigTransaction = facade.transactionFactory.create({
				type: 'multisig_transaction_v1',
				signerPublicKey,
				fee: 25n,
				deadline: 12345n,

				innerTransaction: facade.transactionFactory.static.toNonVerifiableTransaction(transaction)
			});

			return {
				transaction: convertTransactionSdkJsonToRestJson(multisigTransaction.toJson()),
				meta: { hash: { data: TRANSACTION_HASH } }
			};
		};

		const assertIncludeMempoolAccountTest = async (transactions, expectedCoins, includeMempool = true) => {
			// Arrange:
			stubAccountResolutions();
			stubFetchResult(`account/unconfirmedTransactions?address=${ACCOUNT_ADDRESS}`, true, transactions);

			// - create request
			const request = createValidRequest();
			request.include_mempool = includeMempool;

			// - create expected response
			const expectedResponse = new AccountCoinsResponse(
				new BlockIdentifier(12345, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076'),
				expectedCoins
			);

			// Act + Assert:
			await assertRosettaSuccessBasic('/account/coins', request, expectedResponse);
		};

		const createUnconfirmedTransactionsResponse = transactionJsons => ({
			data: transactionJsons
		});

		it('succeeds when all fetches succeed and includes mempool', async () => {
			// Arrange:
			const mosaicsToReceive = [
				{ namespaceId: 'cat', name: 'dog', amount: 200 }
			];
			const expectedResponse = [
				createRosettaCoin('123', 'foo:bar', 3),
				createRosettaCoin('462', 'cat:dog', 4),
				createRosettaCoin('112', 'nem:xem', 6)
			];
			const transactionJson = createTransactionJson(mosaicsToReceive, OTHER_ACCOUNT_PUBLIC_KEY, ACCOUNT_ADDRESS);
			const unconfirmedTransactionsJson = createUnconfirmedTransactionsResponse([transactionJson]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(unconfirmedTransactionsJson, expectedResponse);
		});

		it('succeeds when receive multiple coins in the mempool', async () => {
			// Arrange:
			const mosaicsToReceive = [
				{ namespaceId: 'cat', name: 'dog', amount: 50 },
				{ namespaceId: 'nem', name: 'xem', amount: 150 }
			];
			const expectedResponse = [
				createRosettaCoin('123', 'foo:bar', 3),
				createRosettaCoin('312', 'cat:dog', 4),
				createRosettaCoin('262', 'nem:xem', 6)
			];
			const transactionJson = createTransactionJson(mosaicsToReceive, OTHER_ACCOUNT_PUBLIC_KEY, ACCOUNT_ADDRESS);
			const unconfirmedTransactionsJson = createUnconfirmedTransactionsResponse([transactionJson]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(unconfirmedTransactionsJson, expectedResponse);
		});

		it('succeeds when receive new coin in the mempool', async () => {
			// Arrange:
			const mosaicsToReceive = [
				{ namespaceId: 'cat', name: 'dog', amount: 10 },
				{ namespaceId: 'new', name: 'coin', amount: 50 }
			];
			const expectedResponse = [
				createRosettaCoin('123', 'foo:bar', 3),
				createRosettaCoin('272', 'cat:dog', 4),
				createRosettaCoin('112', 'nem:xem', 6),
				createRosettaCoin('50', 'new:coin', 5)
			];
			const transactionJson = createTransactionJson(mosaicsToReceive, OTHER_ACCOUNT_PUBLIC_KEY, ACCOUNT_ADDRESS);
			const unconfirmedTransactionsJson = createUnconfirmedTransactionsResponse([transactionJson]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(unconfirmedTransactionsJson, expectedResponse);
		});

		it('succeeds when coin sent in the mempool (includes fee)', async () => {
			// Arrange:
			const mosaicsToSend = [
				{ namespaceId: 'cat', name: 'dog', amount: 1 }
			];
			const expectedResponse = [
				createRosettaCoin('123', 'foo:bar', 3),
				createRosettaCoin('261', 'cat:dog', 4),
				createRosettaCoin('62', 'nem:xem', 6)
			];
			const transactionJson = createTransactionJson(mosaicsToSend, ACCOUNT_PUBLIC_KEY, OTHER_ACCOUNT_ADDRESS);
			const unconfirmedTransactionsJson = createUnconfirmedTransactionsResponse([transactionJson]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(unconfirmedTransactionsJson, expectedResponse);
		});

		it('succeeds when multiple coins sent in the mempool (include fee)', async () => {
			// Arrange:
			const mosaicsToSend = [
				{ namespaceId: 'cat', name: 'dog', amount: 200 },
				{ namespaceId: 'nem', name: 'xem', amount: 2 }
			];
			const expectedResponse = [
				createRosettaCoin('123', 'foo:bar', 3),
				createRosettaCoin('62', 'cat:dog', 4),
				createRosettaCoin('60', 'nem:xem', 6)
			];
			const transactionJson = createTransactionJson(mosaicsToSend, ACCOUNT_PUBLIC_KEY, OTHER_ACCOUNT_ADDRESS);
			const unconfirmedTransactionsJson = createUnconfirmedTransactionsResponse([transactionJson]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(unconfirmedTransactionsJson, expectedResponse);
		});

		it('succeeds when multiple transactions receive in the mempool', async () => {
			// Arrange:
			const mosaicsToSend = [
				{ namespaceId: 'cat', name: 'dog', amount: 50 },
				{ namespaceId: 'nem', name: 'xem', amount: 2 }
			];
			const mosaicsToReceive = [
				{ namespaceId: 'cat', name: 'dog', amount: 50 },
				{ namespaceId: 'nem', name: 'xem', amount: 20 },
				{ namespaceId: 'foo', name: 'bar', amount: 23 }
			];
			const expectedResponse = [
				createRosettaCoin('146', 'foo:bar', 3),
				createRosettaCoin('262', 'cat:dog', 4),
				createRosettaCoin('80', 'nem:xem', 6)
			];
			const outgoingTransactionJson = createTransactionJson(mosaicsToSend, ACCOUNT_PUBLIC_KEY, OTHER_ACCOUNT_ADDRESS);
			const incomingTransactionJson = createTransactionJson(mosaicsToReceive, OTHER_ACCOUNT_PUBLIC_KEY, ACCOUNT_ADDRESS);
			const unconfirmedTransactionsJson = createUnconfirmedTransactionsResponse([outgoingTransactionJson, incomingTransactionJson]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(unconfirmedTransactionsJson, expectedResponse);
		});

		it('succeeds when include mempool is false', async () => {
			// Arrange:
			const mosaicsToSend = [
				{ namespaceId: 'cat', name: 'dog', amount: 3 },
				{ namespaceId: 'nem', name: 'xem', amount: 3 }
			];
			const expectedResponse = [
				createRosettaCoin('123', 'foo:bar', 3),
				createRosettaCoin('262', 'cat:dog', 4),
				createRosettaCoin('112', 'nem:xem', 6)
			];
			const transactionJson = createTransactionJson(mosaicsToSend, ACCOUNT_PUBLIC_KEY, OTHER_ACCOUNT_ADDRESS);
			const unconfirmedTransactionsJson = createUnconfirmedTransactionsResponse([transactionJson]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(unconfirmedTransactionsJson, expectedResponse, false);
		});

		it('succeeds when multisig transaction is in mempool', async () => {
			// Arrange:
			const mosaicsToSend = [
				{ namespaceId: 'cat', name: 'dog', amount: 200 },
				{ namespaceId: 'nem', name: 'xem', amount: 2 }
			];
			const expectedResponse = [
				createRosettaCoin('123', 'foo:bar', 3),
				createRosettaCoin('62', 'cat:dog', 4),
				createRosettaCoin('35', 'nem:xem', 6)
			];
			const multisigTransactionJson = createAggregateTransactionJson(mosaicsToSend, ACCOUNT_PUBLIC_KEY, OTHER_ACCOUNT_ADDRESS);
			const unconfirmedTransactionsJson = createUnconfirmedTransactionsResponse([multisigTransactionJson]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(unconfirmedTransactionsJson, expectedResponse);
		});
	});

	// endregion
});
