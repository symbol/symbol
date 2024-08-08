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
	calculateXemTransferFee,
	createLookupCurrencyFunction,
	getBlockchainDescriptor,
	mosaicIdToString
} from '../../../../src/plugins/rosetta/nem/rosettaUtils.js';
import Currency from '../../../../src/plugins/rosetta/openApi/model/Currency.js';
import { expect } from 'chai';

describe('NEM rosetta utils', () => {
	// region getBlockchainDescriptor

	describe('getBlockchainDescriptor', () => {
		it('extracts blockchain descriptor from config', () => {
			// Act:
			const blockchainDescriptor = getBlockchainDescriptor({ network: { name: 'alpha' } });

			// Assert:
			expect(blockchainDescriptor).to.deep.equal({
				blockchain: 'NEM',
				network: 'alpha'
			});
		});
	});

	// endregion

	// region mosaicIdToString

	describe('mosaicIdToString', () => {
		it('can convert mosaic id object to string', () => {
			// Act:
			const str = mosaicIdToString({ namespaceId: 'foo', name: 'bar' });

			// Assert:
			expect(str).to.equal('foo.bar');
		});
	});

	// endregion

	// region calculateXemTransferFee

	describe('calculateXemTransferFee', () => {
		it('can calculate correct fees', () => {
			expect(calculateXemTransferFee(500000n)).to.equal(50000n * 25n);
			expect(calculateXemTransferFee(250000n)).to.equal(50000n * 25n);
			expect(calculateXemTransferFee(200000n)).to.equal(50000n * 20n);
			expect(calculateXemTransferFee(201111n)).to.equal(50000n * 20n);
			expect(calculateXemTransferFee(10000n)).to.equal(50000n * 1n);
			expect(calculateXemTransferFee(1n)).to.equal(50000n * 1n);
		});
	});

	// endregion

	// region createLookupCurrencyFunction

	describe('createLookupCurrencyFunction', () => {
		const mockProxy = {
			mosaicProperties: mosaicId => {
				if ('coins' === mosaicId.name)
					return Promise.resolve({ divisibility: 3, levy: undefined });

				if ('coupons' === mosaicId.name) {
					return Promise.resolve({
						divisibility: 4,
						levy: {
							mosaicId: { namespaceId: 'some.other', name: 'tax' },
							recipientAddress: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
							isAbsolute: true,
							fee: 10
						}
					});
				}

				return Promise.resolve({ divisibility: 2, levy: undefined });
			}
		};

		it('can lookup currency mosaic id', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency('currencyMosaicId');

			// Assert:
			const expectedCurrency = new Currency('nem.xem', 6);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.equal(undefined);
		});

		it('can lookup arbitrary mosaic id (without levy)', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency({ namespaceId: 'foo.bar', name: 'coins' });

			// Assert:
			const expectedCurrency = new Currency('foo.bar.coins', 3);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.equal(undefined);
		});

		it('can lookup arbitrary mosaic id (with levy)', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency({ namespaceId: 'foo.bar', name: 'coupons' });

			// Assert:
			const expectedCurrency = new Currency('foo.bar.coupons', 4);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.deep.equal({
				currency: new Currency('some.other.tax', 2),
				recipientAddress: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
				isAbsolute: true,
				fee: 10
			});
		});
	});

	// endregion
});
