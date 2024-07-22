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
import accountRoutes from '../../../src/plugins/rosetta/accountRoutes.js';
import AccountBalanceResponse from '../../../src/plugins/rosetta/openApi/model/AccountBalanceResponse.js';
import AccountCoinsResponse from '../../../src/plugins/rosetta/openApi/model/AccountCoinsResponse.js';
import Amount from '../../../src/plugins/rosetta/openApi/model/Amount.js';
import BlockIdentifier from '../../../src/plugins/rosetta/openApi/model/BlockIdentifier.js';
import Coin from '../../../src/plugins/rosetta/openApi/model/Coin.js';
import CoinIdentifier from '../../../src/plugins/rosetta/openApi/model/CoinIdentifier.js';
import { RosettaErrorFactory } from '../../../src/plugins/rosetta/rosettaUtils.js';

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
			createRosettaAmount: (...args) => {
				const amount = createRosettaAmount(...args);
				delete amount.currency.metadata;
				return amount;
			},
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

		addAccountSuccessTests('/account/coins', {
			createValidRequest,
			createRosettaAmount: createRosettaCoin,
			Response: AccountCoinsResponse
		});
	});

	// endregion
});
