import ByteArray from '../src/ByteArray.js';
import { uint8ToHex } from '../src/utils/converter.js';
import { expect } from 'chai';
import crypto from 'crypto';

describe('ByteArray', () => {
	const FIXED_SIZE = 24;
	const TEST_BYTES = new Uint8Array([
		0xC5, 0xFB, 0x65, 0xCB, 0x90, 0x26, 0x23, 0xD9,
		0x3D, 0xF2, 0xE6, 0x82, 0xFF, 0xB1, 0x3F, 0x99,
		0xD5, 0x0F, 0xAC, 0x24, 0xD5, 0xFF, 0x2A, 0x42
	]);
	const TEST_HEX = 'C5FB65CB902623D93DF2E682FFB13F99D50FAC24D5FF2A42';

	it('can create byte array with correct number of bytes', () => {
		// Act:
		const byteArray = new ByteArray(FIXED_SIZE, TEST_BYTES);

		// Assert:
		expect(byteArray.bytes).to.deep.equal(TEST_BYTES);
	});

	it('cannot create byte array with incorrect number of bytes', () => {
		[0, FIXED_SIZE - 1, FIXED_SIZE + 1].forEach(size => {
			expect(() => {
				new ByteArray(FIXED_SIZE, crypto.randomBytes(size)); // eslint-disable-line no-new
			}).to.throw('bytes was size');
		});
	});

	it('can create byte array with correct number of hex characters', () => {
		// Act:
		const byteArray = new ByteArray(FIXED_SIZE, TEST_HEX);

		// Assert:
		expect(byteArray.bytes).to.deep.equal(TEST_BYTES);
	});

	it('cannot create byte array with incorrect number of hex characters', () => {
		[0, FIXED_SIZE - 1, FIXED_SIZE + 1].forEach(size => {
			expect(() => {
				new ByteArray(FIXED_SIZE, uint8ToHex(crypto.randomBytes(size))); // eslint-disable-line no-new
			}).to.throw('bytes was size');
		});
	});

	it('supports toString', () => {
		expect(new ByteArray(FIXED_SIZE, TEST_BYTES).toString()).to.equal(TEST_HEX);
	});
});
