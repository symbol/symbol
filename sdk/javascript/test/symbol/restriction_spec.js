import { mosaicRestrictionGenerateKey } from '../../src/symbol/restriction.js';
import { expect } from 'chai';

describe('restriction', () => {
	describe('mosaicRestrictionGenerateKey', () => {
		const assertKeyGeneration = (seed, expectedKey) => {
			// Act:
			const key = mosaicRestrictionGenerateKey(seed);

			// Assert:
			expect(key).to.equal(expectedKey);
		};

		it('can generate expected keys from seeds', () => {
			assertKeyGeneration('a', 0x7524A0FBF24B0880n); // unlike metadataGenerateKey, high bit can be unset
			assertKeyGeneration('abc', 0xB225E24FA75D983An);
			assertKeyGeneration('def', 0xB0AC5222678F0D8En);
		});
	});
});
