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

import ConstructionDeriveRequest from '../../../src/plugins/rosetta/openApi/model/ConstructionDeriveRequest.js';
import Currency from '../../../src/plugins/rosetta/openApi/model/Currency.js';
import RosettaApiError from '../../../src/plugins/rosetta/openApi/model/Error.js';
import {
	RosettaErrorFactory, createLookupCurrencyFunction, rosettaPostRouteWithNetwork, stitchBlockTransactions
} from '../../../src/plugins/rosetta/rosettaUtils.js';
import { expect } from 'chai';

describe('rosetta utils', () => {
	// region RosettaErrorFactory

	describe('RosettaErrorFactory', () => {
		it('all have unique codes', () => {
			// Arrange: filter out properties added to every class
			const defaultClassPropertyNames = Object.getOwnPropertyNames(class C {});
			const errorNames = Object.getOwnPropertyNames(RosettaErrorFactory)
				.filter(name => !defaultClassPropertyNames.includes(name));

			// Act:
			const errorCodeSet = new Set(errorNames.map(name => RosettaErrorFactory[name].apiError.code));

			// Assert:
			expect(errorCodeSet.size).to.equal(errorNames.length);
		});
	});

	// endregion

	// region rosettaPostRouteWithNetwork

	describe('rosettaPostRouteWithNetwork', () => {
		// use /construction/derive types in these tests

		const createValidRequest = () => ({
			network_identifier: {
				blockchain: 'Symbol',
				network: 'testnet'
			},
			public_key: {
				hex_bytes: 'E85D10BF47FFBCE2230F70CB43ED2DDE04FCF342524B383972F86EA1FF773C79',
				curve_type: 'edwards25519'
			}
		});

		const createRosettaRouteTestSetup = () => {
			const routeContext = { numNextCalls: 0 };
			const next = () => { ++routeContext.numNextCalls; };

			routeContext.responses = [];
			routeContext.headers = [];
			const res = {
				statusCode: 200,
				send: response => { routeContext.responses.push(response); },
				setHeader: (name, value) => { routeContext.headers.push({ name, value }); }
			};

			return { routeContext, next, res };
		};

		const assertRosettaErrorRaised = (routeContext, res, expectedError) => {
			expect(routeContext.numNextCalls).to.equal(1);
			expect(routeContext.responses.length).to.equal(1);
			expect(res.statusCode).to.equal(500);

			const response = routeContext.responses[0];
			expect(response).to.deep.equal(expectedError.apiError);
			expect(() => RosettaApiError.validateJSON(response)).to.not.throw();
		};

		it('fails when request is invalid', async () => {
			// Arrange: corrupt the request by removing a required subfield
			const { routeContext, next, res } = createRosettaRouteTestSetup();
			const request = createValidRequest();
			delete request.network_identifier.network;

			// Act:
			const postHandler = rosettaPostRouteWithNetwork('testnet', ConstructionDeriveRequest, () => {});
			await postHandler({ body: request }, res, next);

			// Assert:
			assertRosettaErrorRaised(routeContext, res, RosettaErrorFactory.INVALID_REQUEST_DATA);
		});

		it('fails when network is invalid', async () => {
			// Arrange:
			const { routeContext, next, res } = createRosettaRouteTestSetup();
			const request = createValidRequest();

			// Act:
			const postHandler = rosettaPostRouteWithNetwork('mainnet', ConstructionDeriveRequest, () => {});
			await postHandler({ body: request }, res, next);

			// Assert:
			assertRosettaErrorRaised(routeContext, res, RosettaErrorFactory.UNSUPPORTED_NETWORK);
		});

		const assertFailsWhenHandlerRaisesError = async (raisedError, expectedError) => {
			// Arrange:
			const { routeContext, next, res } = createRosettaRouteTestSetup();
			const request = createValidRequest();

			// Act:
			const postHandler = rosettaPostRouteWithNetwork('testnet', ConstructionDeriveRequest, () => Promise.reject(raisedError));
			await postHandler({ body: request }, res, next);

			// Assert:
			assertRosettaErrorRaised(routeContext, res, expectedError);
		};

		it('fails when handler raises error (rosetta)', () => assertFailsWhenHandlerRaisesError(
			RosettaErrorFactory.INVALID_PUBLIC_KEY,
			RosettaErrorFactory.INVALID_PUBLIC_KEY
		));

		it('fails when handler raises error (unknown)', () => assertFailsWhenHandlerRaisesError(
			Error('unknown error'),
			RosettaErrorFactory.INTERNAL_SERVER_ERROR
		));

		const assertSuccessWhenValid = async handler => {
			// Arrange:
			const { routeContext, next, res } = createRosettaRouteTestSetup();
			const request = createValidRequest();

			// Act:
			const postHandler = rosettaPostRouteWithNetwork('testnet', ConstructionDeriveRequest, handler);
			await postHandler({ body: request }, res, next);

			// Assert:
			expect(routeContext.numNextCalls).to.equal(1);
			expect(routeContext.responses.length).to.equal(1);
			expect(res.statusCode).to.equal(200);
			expect(routeContext.responses[0]).to.deep.equal({ foo: 'testnet' });
		};

		it('succeeds when network is valid', () => assertSuccessWhenValid(typedRequest => ({
			foo: typedRequest.network_identifier.network
		})));

		it('succeeds when network is valid (async)', () => assertSuccessWhenValid(typedRequest => (Promise.resolve({
			foo: typedRequest.network_identifier.network
		}))));
	});

	// endregion

	// region createLookupCurrencyFunction

	describe('createLookupCurrencyFunction', () => {
		const mockProxy = {
			networkProperties: () => Promise.resolve({ chain: { currencyMosaicId: '0x1234567812345678' } }),
			resolveMosaicId: (unresolvedMosaicId, transactionLocation) =>
				unresolvedMosaicId + BigInt(transactionLocation.primaryId + (transactionLocation.secondaryId * 0x100)),
			mosaicProperties: mosaicId => Promise.resolve({
				id: mosaicId.toString(16).toUpperCase(),
				name: 'foo.bar',
				divisibility: 3
			})
		};

		it('can lookup currency mosaic id', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const currency = await lookupCurrency('currencyMosaicId', { primaryId: 0x11, secondaryId: 0x246 });

			// Assert:
			const expectedCurrency = new Currency('foo.bar', 3);
			expectedCurrency.metadata = { id: '1234567812345678' };
			expect(currency).to.deep.equal(expectedCurrency);
		});

		it('can lookup arbitrary mosaic id', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const currency = await lookupCurrency(0x1111222233334444n, { primaryId: 0x11, secondaryId: 0x246 });

			// Assert:
			const expectedCurrency = new Currency('foo.bar', 3);
			expectedCurrency.metadata = { id: '1111222233358A55' };
			expect(currency).to.deep.equal(expectedCurrency);
		});
	});

	// endregion

	// region stitchBlockTransactions

	describe('stitchBlockTransactions', () => {
		it('does not modify regular transactions', () => {
			// Act:
			const transactions = stitchBlockTransactions([
				{ transaction: { tag: 'alpha' }, meta: { hash: 'A' } },
				{ transaction: { tag: 'beta' }, meta: { hash: 'B' } },
				{ transaction: { tag: 'gamma' }, meta: { hash: 'C' } }
			]);

			// Assert:
			expect(transactions).to.deep.equal([
				{ transaction: { tag: 'alpha' }, meta: { hash: 'A' } },
				{ transaction: { tag: 'beta' }, meta: { hash: 'B' } },
				{ transaction: { tag: 'gamma' }, meta: { hash: 'C' } }
			]);
		});

		it('stitches embedded transactions into their containing aggregate transactions', () => {
			// Act:
			const transactions = stitchBlockTransactions([
				{ transaction: { tag: 'alpha' }, meta: { hash: 'A' } },
				{ transaction: { tag: 'beta' }, meta: { hash: 'B' } },
				{ transaction: { tag: 'gamma' }, meta: { hash: 'C' } },
				{ transaction: { tag: 'omega' }, meta: { aggregateHash: 'A', index: 2 } },
				{ transaction: { tag: 'sigma' }, meta: { aggregateHash: 'C', index: 1 } },
				{ transaction: { tag: 'psi' }, meta: { aggregateHash: 'D', index: 0 } }, // dropped
				{ transaction: { tag: 'zeta' }, meta: { aggregateHash: 'A', index: 0 } }
			]);

			// Assert:
			expect(transactions).to.deep.equal([
				{
					transaction: {
						tag: 'alpha',
						transactions: [
							{ transaction: { tag: 'zeta' }, meta: { aggregateHash: 'A', index: 0 } },
							{ transaction: { tag: 'omega' }, meta: { aggregateHash: 'A', index: 2 } }
						]
					},
					meta: { hash: 'A' }
				},
				{ transaction: { tag: 'beta' }, meta: { hash: 'B' } },
				{
					transaction: {
						tag: 'gamma',
						transactions: [
							{ transaction: { tag: 'sigma' }, meta: { aggregateHash: 'C', index: 1 } }
						]
					},
					meta: { hash: 'C' }
				}
			]);
		});
	});

	// endregion
});
