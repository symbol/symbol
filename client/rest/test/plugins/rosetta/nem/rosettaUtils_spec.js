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
	areMosaicDefinitionsEqual,
	areMosaicIdsEqual,
	calculateXemTransferFee,
	createLookupCurrencyFunction,
	getBlockchainDescriptor,
	isMosaicDefinitionDescriptionSignificant,
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
			expect(str).to.equal('foo:bar');
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
			mosaicProperties: (mosaicId, transactionLocation) => {
				if ('coins' === mosaicId.name)
					return Promise.resolve({ divisibility: transactionLocation.height, levy: undefined });

				if ('coupons' === mosaicId.name) {
					const levyMosaicId = 'real.bar' === mosaicId.namespaceId
						? { namespaceId: 'nem', name: 'xem' }
						: { namespaceId: 'some.other', name: 'tax' };
					return Promise.resolve({
						divisibility: transactionLocation.height * 2,
						levy: {
							mosaicId: levyMosaicId,
							recipientAddress: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
							isAbsolute: true,
							fee: transactionLocation.height * 100
						}
					});
				}

				return Promise.resolve({ divisibility: transactionLocation.height, levy: undefined });
			}
		};

		it('can lookup currency mosaic id', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency('currencyMosaicId');

			// Assert:
			const expectedCurrency = new Currency('nem:xem', 6);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.equal(undefined);
		});

		it('can lookup nem.xem', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency({ namespaceId: 'nem', name: 'xem' });

			// Assert: bypasses mosaicProperties
			const expectedCurrency = new Currency('nem:xem', 6);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.equal(undefined);
		});

		it('can lookup nem.other', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency({ namespaceId: 'nem', name: 'other' }, { height: 2 });

			// Assert: does not bypass mosaicProperties
			const expectedCurrency = new Currency('nem:other', 2);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.equal(undefined);
		});

		it('can lookup arbitrary mosaic id (without levy)', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency({ namespaceId: 'foo.bar', name: 'coins' }, { height: 3 });

			// Assert:
			const expectedCurrency = new Currency('foo.bar:coins', 3);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.equal(undefined);
		});

		it('can lookup arbitrary mosaic id (with levy)', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency({ namespaceId: 'foo.bar', name: 'coupons' }, { height: 2 });

			// Assert:
			const expectedCurrency = new Currency('foo.bar:coupons', 4);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.deep.equal({
				currency: new Currency('some.other:tax', 2),
				recipientAddress: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
				isAbsolute: true,
				fee: 200
			});
		});

		it('can lookup arbitrary mosaic id (with nem.xem levy)', async () => {
			// Act:
			const lookupCurrency = createLookupCurrencyFunction(mockProxy);
			const { currency, levy } = await lookupCurrency({ namespaceId: 'real.bar', name: 'coupons' }, { height: 2 });

			// Assert:
			const expectedCurrency = new Currency('real.bar:coupons', 4);
			expect(currency).to.deep.equal(expectedCurrency);
			expect(levy).to.deep.equal({
				currency: new Currency('nem:xem', 6),
				recipientAddress: 'TD3RXTHBLK6J3UD2BH2PXSOFLPWZOTR34WCG4HXH',
				isAbsolute: true,
				fee: 200
			});
		});
	});

	// endregion

	// region areMosaicIdsEqual

	describe('areMosaicIdsEqual', () => {
		const createMosaicIdJson = () => ({ namespaceId: 'foo', name: 'bar' });

		const runAreEqualTest = (expectedAreEqual, transform) => {
			// Arrange:
			const mosaicId1 = createMosaicIdJson();
			const mosaicId2 = createMosaicIdJson();
			transform(mosaicId2);

			// Act:
			const areEqual = areMosaicIdsEqual(mosaicId1, mosaicId2);

			// Assert:
			expect(areEqual).to.equal(expectedAreEqual);
		};

		it('is true when everything matches', () => runAreEqualTest(true, () => {}));

		it('is false when namespaceId does not match', () => runAreEqualTest(false, mosaicId => {
			mosaicId.namespaceId = 'baz';
		}));

		it('is false when name does not match', () => runAreEqualTest(false, mosaicId => {
			mosaicId.name = 'baz';
		}));
	});

	// endregion

	// region areMosaicDefinitionsEqual

	describe('areMosaicDefinitionsEqual', () => {
		const createMosaicDefinitionJson = () => ({
			creator: '345E4422E20696EC7B77DDB52F7650884ADFA2FE5BCF2571A86725D4424A9F53',
			description: 'my mosaic',
			id: { namespaceId: 'foo', name: 'tokens' },
			properties: [
				{ name: 'divisibility', value: '1' },
				{ name: 'initialSupply', value: '2' },
				{ name: 'supplyMutable', value: 'true' },
				{ name: 'transferable', value: 'false' }
			],
			levy: {
				fee: 5000000,
				recipient: 'TAJYBYLBMEVAJUEESLKXQJ2PGKQ7JOWSNT7VQAN7',
				type: 1,
				mosaicId: { namespaceId: 'foo', name: 'tax' }
			}
		});

		const runAreEqualTest = (isDescriptionSignificant, expectedAreEqual, transform) => {
			// Arrange:
			const mosaicDefinition1 = createMosaicDefinitionJson();
			const mosaicDefinition2 = createMosaicDefinitionJson();
			transform(mosaicDefinition2);

			// Act:
			const areEqual = areMosaicDefinitionsEqual(mosaicDefinition1, mosaicDefinition2, isDescriptionSignificant);

			// Assert:
			expect(areEqual).to.equal(expectedAreEqual);
		};

		it('is true when everything matches', () => runAreEqualTest(true, true, () => {}));

		it('is true when everything matches except description (insignificant)', () => runAreEqualTest(false, true, mosaicDefinition => {
			mosaicDefinition.description = 'foo tokens';
		}));

		it('is false when everything matches except description (significant)', () => runAreEqualTest(true, false, mosaicDefinition => {
			mosaicDefinition.description = 'foo tokens';
		}));

		it('is false when creator does not match', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.creator = '1EEE06798969312A0D0698F8A9E916E57A23278202440014C6A5DF9674E5206D';
		}));

		it('is false when property does not match (divisibility)', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.properties[0].value = '3';
		}));

		it('is false when property does not match (initialSupply)', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.properties[1].value = '100';
		}));

		it('is false when property does not match (supplyMutable)', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.properties[2].value = 'false';
		}));

		it('is false when property does not match (transferable)', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.properties[3].value = 'true';
		}));

		it('is false when levy does not match (fee)', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.levy.fee += 1;
		}));

		it('is false when levy does not match (recipient)', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.levy.recipient = 'TBYBXDPIUJMT4LCILS5TIDFFJXTYHP3GIMYJMGI';
		}));

		it('is false when levy does not match (type)', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.levy.type += 1;
		}));

		it('is false when levy does not match (mosaicId)', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.levy.mosaicId.namespaceId = 'bar';
		}));

		it('is false when one levy is empty', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.levy = {};
		}));

		it('is false when one levy is undefined', () => runAreEqualTest(false, false, mosaicDefinition => {
			mosaicDefinition.levy = undefined;
		}));
	});

	// endregion

	// region isMosaicDefinitionDescriptionSignificant

	describe('isMosaicDefinitionDescriptionSignificant', () => {
		it('returns correct values for mainnet', () => {
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'mainnet' }, { height: 1 })).to.equal(true);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'mainnet' }, { height: 500_000 })).to.equal(true);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'mainnet' }, { height: 1_109_999 })).to.equal(true);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'mainnet' }, { height: 1_110_000 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'mainnet' }, { height: 1_110_001 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'mainnet' }, { height: 1_500_000 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'mainnet' }, { height: undefined })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'mainnet' }, undefined)).to.equal(false);
		});

		it('returns correct values for testnet', () => {
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'testnet' }, { height: 1 })).to.equal(true);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'testnet' }, { height: 500_000 })).to.equal(true);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'testnet' }, { height: 871_499 })).to.equal(true);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'testnet' }, { height: 871_500 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'testnet' }, { height: 871_501 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'testnet' }, { height: 1_500_000 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'testnet' }, { height: undefined })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'testnet' }, undefined)).to.equal(false);
		});

		it('returns correct values for other network', () => {
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'othernet' }, { height: 1 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'othernet' }, { height: 500_000 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'othernet' }, { height: 1_000_000 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'othernet' }, { height: 1_500_000 })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'othernet' }, { height: undefined })).to.equal(false);
			expect(isMosaicDefinitionDescriptionSignificant({ name: 'othernet' }, undefined)).to.equal(false);
		});
	});

	// endregion
});
