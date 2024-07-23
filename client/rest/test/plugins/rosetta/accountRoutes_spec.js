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
	RosettaOperationFactory,
	assertRosettaErrorRaisedBasicWithRoutes,
	assertRosettaSuccessBasicWithRoutes
} from './utils/rosettaTestUtils.js';
import { convertTransactionSdkJsonToRestJson } from '../../../src/plugins/rosetta/OperationParser.js';
import accountRoutes from '../../../src/plugins/rosetta/accountRoutes.js';
import AccountBalanceResponse from '../../../src/plugins/rosetta/openApi/model/AccountBalanceResponse.js';
import AccountCoinsResponse from '../../../src/plugins/rosetta/openApi/model/AccountCoinsResponse.js';
import Amount from '../../../src/plugins/rosetta/openApi/model/Amount.js';
import BlockIdentifier from '../../../src/plugins/rosetta/openApi/model/BlockIdentifier.js';
import Coin from '../../../src/plugins/rosetta/openApi/model/Coin.js';
import CoinIdentifier from '../../../src/plugins/rosetta/openApi/model/CoinIdentifier.js';
import { RosettaErrorFactory } from '../../../src/plugins/rosetta/rosettaUtils.js';
import { SymbolFacade, generateMosaicAliasId } from 'symbol-sdk/symbol';

describe('account routes', () => {
	const assertRosettaErrorRaisedBasic = (...args) => assertRosettaErrorRaisedBasicWithRoutes(accountRoutes, ...args);
	const assertRosettaSuccessBasic = (...args) => assertRosettaSuccessBasicWithRoutes(accountRoutes, ...args);

	// region utils

	const { createRosettaNetworkIdentifier } = RosettaObjectFactory;

	const createRosettaAmount = (amount, currencyName, currencyDecimals, mosaicId) => new Amount(
		amount,
		RosettaOperationFactory.createCurrency(currencyName, currencyDecimals, mosaicId)
	);

	const createRosettaCoin = (amount, currencyName, currencyDecimals, mosaicId) => new Coin(
		new CoinIdentifier(mosaicId),
		createRosettaAmount(amount, currencyName, currencyDecimals, mosaicId)
	);

	const stubFetchResult = FetchStubHelper.stubPost;
	FetchStubHelper.registerStubCleanup();

	const stubAccountResolutions = () => {
		// setup chain
		stubFetchResult('chain/info', true, { height: '12345' });
		stubFetchResult('blocks/12345', true, {
			block: { height: '12345' },
			meta: { hash: 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076' }
		});

		// setup account
		stubFetchResult('accounts/TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', true, {
			account: {
				mosaics: [
					{ id: '1122334455667788', amount: '123' },
					{ id: '0F61E4A360897965', amount: '262' },
					{ id: '1ABBCCDDAABBCCDD', amount: '112' }
				]
			}
		});

		// resolve mosaics
		FetchStubHelper.stubMosaicResolution('1122334455667788', 'foo.bar', 3);
		FetchStubHelper.stubMosaicResolution('0F61E4A360897965', 'cat.dog', 4);
		FetchStubHelper.stubMosaicResolution('1ABBCCDDAABBCCDD', 'symbol.xym', 6);
		FetchStubHelper.stubMosaicResolution('2345678901ABBCCD', 'new.coin', 5);
	};

	const bigIntToHexString = value => value.toString(16).padStart(16, '0').toUpperCase();

	const stubNamespaceResolution = (namespaceName, mosaicId) => {
		const namespaceId = bigIntToHexString(generateMosaicAliasId(namespaceName));
		stubFetchResult(`namespaces/${namespaceId}`, true, { namespace: { alias: { mosaicId } } });
	};

	const stubNamespaceResolutions = () => {
		stubNamespaceResolution('symbol.xym', '1ABBCCDDAABBCCDD');
		stubNamespaceResolution('foo.bar', '1122334455667788');
		stubNamespaceResolution('cat.dog', '0F61E4A360897965');
		stubNamespaceResolution('new.coin', '2345678901ABBCCD');
	};

	const createTransactionJson = (mosaicList, signerPublicKey, recipientAddress) => {
		const facade = new SymbolFacade('testnet');
		const transaction = facade.transactionFactory.create({
			type: 'transfer_transaction_v1',
			recipientAddress,
			signerPublicKey,
			mosaics: [
				...mosaicList
			],
			fee: 100
		});

		return {
			transaction: convertTransactionSdkJsonToRestJson(transaction.toJson()),
			meta: {
				hash: 'C65DF0B9CB47E1D3538DC40481FC613F37DA4DEE816F72FDF63061B2707F6483',
				height: '0',
				index: 1
			}
		};
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
				stubFetchResult('chain/info', true, { height: '12345' });
				stubFetchResult('chain/info', true, { height: '12346' }, undefined, 'Second');
				stubFetchResult('accounts/TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', true, { account: { mosaics: [] } });
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
					options.createRosettaAmount('123', 'foo.bar', 3, '1122334455667788'),
					options.createRosettaAmount('262', 'cat.dog', 4, '0F61E4A360897965'),
					options.createRosettaAmount('112', 'symbol.xym', 6, '1ABBCCDDAABBCCDD')
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
				{ symbol: 'foo.baz', decimals: 3 }, // name mismatch
				{ symbol: 'cat.dog', decimals: 4 },
				{ symbol: 'symbol.xym', decimals: 1 } // decimals mismatch
			];

			// - create expected response
			const expectedResponse = new options.Response(
				new BlockIdentifier(12345, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076'),
				[
					options.createRosettaAmount('262', 'cat.dog', 4, '0F61E4A360897965')
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
			account_identifier: { address: 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI' }
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
			account_identifier: { address: 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI' },
			include_mempool: false
		});

		addAccountFailureTests('/account/coins', {
			createValidRequest,
			skipBlockIdentifierTest: true
		});

		const incomingSignerPublicKey = '527068DA90B142D98D27FF9BA2103A54230E3C8FAC8529E804123D986CACDCC9';
		const incomingRecipientAddress = 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI';
		const signerPublicKey = 'ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6';
		const recipientAddress = 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI';

		const assertIncludeMempoolAccountTest = async (transactionList, expectedCoins, include_mempool = true) => {
			// Arrange:
			stubAccountResolutions();
			stubFetchResult(
				'transactions/unconfirmed?embedded=true&pageNumber=1&pageSize=100',
				true,
				transactionList
			);

			// - resolve namespace id
			stubNamespaceResolutions();

			// - resolve 'currencyMosaicId'
			FetchStubHelper.stubCatapultProxyCacheFill();
			stubFetchResult('network/properties', true, { chain: { currencyMosaicId: '0x1ABBCCDDAABBCCDD' } });

			// - add currency filter
			const request = createValidRequest();
			request.include_mempool = include_mempool;

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

		it('succeeds when all fetches succeed', async () => {
			// Arrange:
			stubAccountResolutions();

			// - create expected response
			const expectedResponse = new AccountCoinsResponse(
				new BlockIdentifier(12345, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076'),
				[
					createRosettaCoin('123', 'foo.bar', 3, '1122334455667788'),
					createRosettaCoin('262', 'cat.dog', 4, '0F61E4A360897965'),
					createRosettaCoin('112', 'symbol.xym', 6, '1ABBCCDDAABBCCDD')
				]
			);

			// Act + Assert:
			await assertRosettaSuccessBasic('/account/coins', createValidRequest(), expectedResponse);
		});

		it('succeeds when all fetches succeed and applies currency filter', async () => {
			// Arrange:
			stubAccountResolutions();

			// - add currency filter
			const request = createValidRequest();
			request.currencies = [
				{ symbol: 'foo.baz', decimals: 3 }, // name mismatch
				{ symbol: 'cat.dog', decimals: 4 },
				{ symbol: 'symbol.xym', decimals: 1 } // decimals mismatch
			];

			// - create expected response
			const expectedResponse = new AccountCoinsResponse(
				new BlockIdentifier(12345, 'A4950F27A23B235D5CCD1DC7FF4B0BDC48977E353EA1CF1E3E5F70B9A6B79076'),
				[
					createRosettaCoin('262', 'cat.dog', 4, '0F61E4A360897965')
				]
			);

			// Act + Assert:
			await assertRosettaSuccessBasic('/account/coins', request, expectedResponse);
		});

		it('succeeds when all fetches succeed and includes mempool', async () => {
			// Arrange:
			const mosaicsList = [
				{ mosaicId: generateMosaicAliasId('cat.dog'), amount: 100 }
			];
			const expectedCoins = [
				createRosettaCoin('123', 'foo.bar', 3, '1122334455667788'),
				createRosettaCoin('362', 'cat.dog', 4, '0F61E4A360897965'),
				createRosettaCoin('112', 'symbol.xym', 6, '1ABBCCDDAABBCCDD')
			];
			const transaction = createTransactionJson(mosaicsList, incomingSignerPublicKey, incomingRecipientAddress);
			const transactionJson = createUnconfirmedTransactionsResponse([transaction]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(transactionJson, expectedCoins);
		});

		it('succeeds when receive multiple coins in the mempool', async () => {
			// Arrange:
			const mosaicsList = [
				{ mosaicId: generateMosaicAliasId('cat.dog'), amount: 100 },
				{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 100 }
			];
			const expectedCoins = [
				createRosettaCoin('123', 'foo.bar', 3, '1122334455667788'),
				createRosettaCoin('362', 'cat.dog', 4, '0F61E4A360897965'),
				createRosettaCoin('212', 'symbol.xym', 6, '1ABBCCDDAABBCCDD')
			];
			const transaction = createTransactionJson(mosaicsList, incomingSignerPublicKey, incomingRecipientAddress);
			const transactionJson = createUnconfirmedTransactionsResponse([transaction]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(transactionJson, expectedCoins);
		});

		it('succeeds when receive new coin in the mempool', async () => {
			// Arrange:
			const mosaicsList = [
				{ mosaicId: generateMosaicAliasId('cat.dog'), amount: 100 },
				{ mosaicId: generateMosaicAliasId('new.coin'), amount: 50 }
			];
			const expectedCoins = [
				createRosettaCoin('123', 'foo.bar', 3, '1122334455667788'),
				createRosettaCoin('362', 'cat.dog', 4, '0F61E4A360897965'),
				createRosettaCoin('112', 'symbol.xym', 6, '1ABBCCDDAABBCCDD'),
				createRosettaCoin('50', 'new.coin', 5, '2345678901ABBCCD')
			];
			const transaction = createTransactionJson(mosaicsList, incomingSignerPublicKey, incomingRecipientAddress);
			const transactionJson = createUnconfirmedTransactionsResponse([transaction]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(transactionJson, expectedCoins);
		});

		it('succeeds when coin sent in the mempool (includes fee)', async () => {
			// Arrange:
			const mosaicsList = [
				{ mosaicId: generateMosaicAliasId('cat.dog'), amount: 100 }
			];
			const expectedCoins = [
				createRosettaCoin('123', 'foo.bar', 3, '1122334455667788'),
				createRosettaCoin('162', 'cat.dog', 4, '0F61E4A360897965'),
				createRosettaCoin('12', 'symbol.xym', 6, '1ABBCCDDAABBCCDD')
			];
			const transaction = createTransactionJson(mosaicsList, signerPublicKey, recipientAddress);
			const transactionJson = createUnconfirmedTransactionsResponse([transaction]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(transactionJson, expectedCoins);
		});

		it('succeeds when multiple coins sent in the mempool (include fee)', async () => {
			// Arrange:
			const mosaicsList = [
				{ mosaicId: generateMosaicAliasId('cat.dog'), amount: 100 },
				{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 2 }
			];
			const expectedCoins = [
				createRosettaCoin('123', 'foo.bar', 3, '1122334455667788'),
				createRosettaCoin('162', 'cat.dog', 4, '0F61E4A360897965'),
				createRosettaCoin('10', 'symbol.xym', 6, '1ABBCCDDAABBCCDD')
			];

			const transaction = createTransactionJson(mosaicsList, signerPublicKey, recipientAddress);
			const transactionJson = createUnconfirmedTransactionsResponse([transaction]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(transactionJson, expectedCoins);
		});

		it('succeeds when multiple transactions receive in the mempool', async () => {
			// Arrange:
			const outgoingMosaicsList = [
				{ mosaicId: generateMosaicAliasId('cat.dog'), amount: 100 },
				{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 2 }
			];
			const incomingMosaicsList = [
				{ mosaicId: generateMosaicAliasId('cat.dog'), amount: 50 },
				{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 20 },
				{ mosaicId: generateMosaicAliasId('foo.bar'), amount: 23 }
			];
			const expectedCoins = [
				createRosettaCoin('146', 'foo.bar', 3, '1122334455667788'),
				createRosettaCoin('212', 'cat.dog', 4, '0F61E4A360897965'),
				createRosettaCoin('30', 'symbol.xym', 6, '1ABBCCDDAABBCCDD')
			];
			const outgoingTransaction = createTransactionJson(outgoingMosaicsList, signerPublicKey, recipientAddress);
			const incomingTransaction = createTransactionJson(incomingMosaicsList, incomingSignerPublicKey, incomingRecipientAddress);
			const transactionJson = createUnconfirmedTransactionsResponse([outgoingTransaction, incomingTransaction]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(transactionJson, expectedCoins);
		});

		it('succeeds when include mempool is false', async () => {
			// Arrange:
			const mosaicsList = [
				{ mosaicId: generateMosaicAliasId('cat.dog'), amount: 100 },
				{ mosaicId: generateMosaicAliasId('symbol.xym'), amount: 2 }
			];
			const expectedCoins = [
				createRosettaCoin('123', 'foo.bar', 3, '1122334455667788'),
				createRosettaCoin('262', 'cat.dog', 4, '0F61E4A360897965'),
				createRosettaCoin('112', 'symbol.xym', 6, '1ABBCCDDAABBCCDD')
			];
			const transaction = createTransactionJson(mosaicsList, signerPublicKey, recipientAddress);
			const transactionJson = createUnconfirmedTransactionsResponse([transaction]);

			// Act + Assert:
			await assertIncludeMempoolAccountTest(transactionJson, expectedCoins, false);
		});
	});

	// endregion
});
