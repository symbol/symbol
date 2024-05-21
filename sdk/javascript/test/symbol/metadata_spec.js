import { metadataGenerateKey, metadataUpdateValue } from '../../src/symbol/metadata.js';
import { expect } from 'chai';

describe('metadata', () => {
	describe('metadataGenerateKey', () => {
		const assertKeyGeneration = (seed, expectedKey) => {
			// Act:
			const key = metadataGenerateKey(seed);

			// Assert:
			expect(key).to.equal(expectedKey);
		};

		it('can generate expected keys from seeds', () => {
			assertKeyGeneration('a', 0xF524A0FBF24B0880n);
			assertKeyGeneration('abc', 0xB225E24FA75D983An);
			assertKeyGeneration('def', 0xB0AC5222678F0D8En);
		});
	});

	describe('metadataUpdateValue', () => {
		it('can set new value without old value', () => {
			// Arrange:
			const newValue = new Uint8Array([0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36]);

			// Act:
			const result = metadataUpdateValue(undefined, newValue);

			// Assert:
			expect(result).to.deep.equal(newValue);
		});

		it('can update equal length value', () => {
			// Arrange:
			const oldValue = new Uint8Array([0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36]);
			const newValue = new Uint8Array([0xD4, 0x60, 0x82, 0xF8, 0x78, 0xFE, 0x78]);

			// Act:
			const result = metadataUpdateValue(oldValue, newValue);

			// Assert:
			expect(result).to.deep.equal(new Uint8Array([
				0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8, 0xA7 ^ 0x78, 0xB0 ^ 0xFE, 0x36 ^ 0x78
			]));
		});

		it('can update shorter value', () => {
			// Arrange:
			const oldValue = new Uint8Array([0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36]);
			const newValue = new Uint8Array([0xD4, 0x60, 0x82, 0xF8]);

			// Act:
			const result = metadataUpdateValue(oldValue, newValue);

			// Assert:
			expect(result).to.deep.equal(new Uint8Array([0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8, 0xA7, 0xB0, 0x36]));
		});

		it('can update longer value', () => {
			// Arrange:
			const oldValue = new Uint8Array([0x9A, 0xC7, 0x33, 0x18, 0xA7, 0xB0, 0x36]);
			const newValue = new Uint8Array([0xD4, 0x60, 0x82, 0xF8, 0x78, 0xFE, 0x78, 0xE6, 0x9D, 0xD6]);

			// Act:
			const result = metadataUpdateValue(oldValue, newValue);

			// Assert:
			expect(result).to.deep.equal(new Uint8Array([
				0x9A ^ 0xD4, 0xC7 ^ 0x60, 0x33 ^ 0x82, 0x18 ^ 0xF8, 0xA7 ^ 0x78, 0xB0 ^ 0xFE, 0x36 ^ 0x78, 0xE6, 0x9D, 0xD6
			]));
		});
	});
});
