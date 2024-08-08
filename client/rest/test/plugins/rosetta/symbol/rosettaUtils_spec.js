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

import Currency from '../../../../src/plugins/rosetta/openApi/model/Currency.js';
import {
	createLookupCurrencyFunction, getBlockchainDescriptor, stitchBlockTransactions
} from '../../../../src/plugins/rosetta/symbol/rosettaUtils.js';
import { expect } from 'chai';

describe('Symbol rosetta utils', () => {
	// region getBlockchainDescriptor

	describe('getBlockchainDescriptor', () => {
		it('extracts blockchain descriptor from config', () => {
			// Act:
			const blockchainDescriptor = getBlockchainDescriptor({ network: { name: 'alpha' } });

			// Assert:
			expect(blockchainDescriptor).to.deep.equal({
				blockchain: 'Symbol',
				network: 'alpha'
			});
		});
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
