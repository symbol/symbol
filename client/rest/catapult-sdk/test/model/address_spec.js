/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const address = require('../../src/model/address');
const convert = require('../../src/utils/convert');
const test = require('../testUtils');
const { expect } = require('chai');

const Address_Decoded_Size = 25;
const Network_Mijin_Identifier = 0x60;
const Network_Public_Test_Identifier = 0x98;

describe('address', () => {
	describe('stringToAddress', () => {
		const assertCannotCreateAddress = (encoded, message) => {
			// Assert:
			expect(() => { address.stringToAddress(encoded); }).to.throw(message);
		};

		it('can create address from valid encoded address', () => {
			// Arrange:
			const encoded = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG';
			const expectedHex = '6823BB7C3C089D996585466380EDBDC19D4959184893E38CA6';

			// Act:
			const decoded = address.stringToAddress(encoded);

			// Assert:
			expect(address.isValidAddress(decoded)).to.equal(true);
			expect(convert.uint8ToHex(decoded)).to.equal(expectedHex);
		});

		it('cannot create address from encoded string with wrong length', () => {
			// Assert:
			assertCannotCreateAddress(
				'NC5J5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV7LASABCDEFGH',
				'NC5J5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV7LASABCDEFGH does not represent a valid encoded address'
			);
		});

		it('cannot create address from invalid encoded string', () => {
			// Assert:
			assertCannotCreateAddress('NC5(5DI2URIC4H3T3IMXQS25PWQWZIPEV6EV7LAS', 'illegal base32 character (');
			assertCannotCreateAddress('NC5J1DI2URIC4H3T3IMXQS25PWQWZIPEV6EV7LAS', 'illegal base32 character 1');
			assertCannotCreateAddress('NC5J5?I2URIC4H3T3IMXQS25PWQWZIPEV6EV7LAS', 'illegal base32 character ?');
		});
	});

	describe('addressToString', () => {
		it('can create encoded address from address', () => {
			// Arrange:
			const decodedHex = '6823BB7C3C089D996585466380EDBDC19D4959184893E38CA6';
			const expected = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG';

			// Act:
			const encoded = address.addressToString(convert.hexToUint8(decodedHex));

			// Assert:
			expect(encoded).to.equal(expected);
		});

		it('cannot create encoded address from invalid address', () => {
			// Arrange:
			const decodedHexStrings = [
				'6823BB7C3C089D996585466380EDBDC19D4959184893E38C', // too short
				'6823BB7C3C089D996585466380EDBDC19D4959184893E38CA678' // too long
			];

			// Act + Assert:
			decodedHexStrings.forEach(decodedHex => {
				expect(() => { address.addressToString(convert.hexToUint8(decodedHex)); }, decodedHex)
					.to.throw(`${decodedHex} does not represent a valid decoded address`);
			});
		});
	});

	describe('publicKeyToAddress', () => {
		it('can create address from public key for well known network', () => {
			// Arrange:
			const expectedHex = '6023BB7C3C089D996585466380EDBDC19D49591848B3727714';
			const publicKey = convert.hexToUint8('3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB');

			// Act:
			const decoded = address.publicKeyToAddress(publicKey, Network_Mijin_Identifier);

			// Assert:
			expect(decoded[0]).to.equal(Network_Mijin_Identifier);
			expect(address.isValidAddress(decoded)).to.equal(true);
			expect(convert.uint8ToHex(decoded)).to.equal(expectedHex);
		});

		it('can create address from public key for custom network', () => {
			// Arrange:
			const expectedHex = '9823BB7C3C089D996585466380EDBDC19D495918484BF7E997';
			const publicKey = convert.hexToUint8('3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB');

			// Act:
			const decoded = address.publicKeyToAddress(publicKey, Network_Public_Test_Identifier);

			// Assert:
			expect(decoded[0]).to.equal(Network_Public_Test_Identifier);
			expect(address.isValidAddress(decoded)).to.equal(true);
			expect(convert.uint8ToHex(decoded)).to.equal(expectedHex);
		});

		it('address calculation is deterministic', () => {
			// Arrange:
			const publicKey = convert.hexToUint8('3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB');

			// Act:
			const decoded1 = address.publicKeyToAddress(publicKey, Network_Mijin_Identifier);
			const decoded2 = address.publicKeyToAddress(publicKey, Network_Mijin_Identifier);

			// Assert:
			expect(address.isValidAddress(decoded1)).to.equal(true);
			expect(decoded1).to.deep.equal(decoded2);
		});

		it('different public keys result in different addresses', () => {
			// Arrange:
			const publicKey1 = test.random.publicKey();
			const publicKey2 = test.random.publicKey();

			// Act:
			const decoded1 = address.publicKeyToAddress(publicKey1, Network_Mijin_Identifier);
			const decoded2 = address.publicKeyToAddress(publicKey2, Network_Mijin_Identifier);

			// Assert:
			expect(address.isValidAddress(decoded1)).to.equal(true);
			expect(address.isValidAddress(decoded2)).to.equal(true);
			expect(decoded1).to.not.deep.equal(decoded2);
		});

		it('different networks result in different addresses', () => {
			// Arrange:
			const publicKey = test.random.publicKey();

			// Act:
			const decoded1 = address.publicKeyToAddress(publicKey, Network_Mijin_Identifier);
			const decoded2 = address.publicKeyToAddress(publicKey, Network_Public_Test_Identifier);

			// Assert:
			expect(address.isValidAddress(decoded1)).to.equal(true);
			expect(address.isValidAddress(decoded2)).to.equal(true);
			expect(decoded1).to.not.deep.equal(decoded2);
		});
	});

	describe('isValidAddress', () => {
		it('returns true for valid address', () => {
			// Arrange:
			const validHex = '6823BB7C3C089D996585466380EDBDC19D4959184893E38CA6';
			const decoded = convert.hexToUint8(validHex);

			// Assert:
			expect(address.isValidAddress(decoded)).to.equal(true);
		});

		it('returns false for address with invalid checksum', () => {
			// Arrange:
			const validHex = '6823BB7C3C089D996585466380EDBDC19D4959184893E38CA6';
			const decoded = convert.hexToUint8(validHex);
			decoded[Address_Decoded_Size - 1] ^= 0xff; // ruin checksum

			// Assert:
			expect(address.isValidAddress(decoded)).to.equal(false);
		});

		it('returns false for address with invalid hash', () => {
			// Arrange:
			const validHex = '6823BB7C3C089D996585466380EDBDC19D4959184893E38CA6';
			const decoded = convert.hexToUint8(validHex);
			decoded[5] ^= 0xff; // ruin ripemd160 hash

			// Assert:
			expect(address.isValidAddress(decoded)).to.equal(false);
		});
	});

	describe('isValidEncodedAddress', () => {
		it('returns true for valid encoded address', () => {
			// Arrange:
			const encoded = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG';

			// Assert:
			expect(address.isValidEncodedAddress(encoded)).to.equal(true);
		});

		it('returns false for invalid encoded address', () => {
			// Arrange:
			const encodedAddresses = [
				'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFH', // changed last char
				'NAR3W7B4BCOZSZMFIZRYx3N5YGOUSWIYJCJ6HDFG' // non-base32 char ('x')
			];

			// Assert:
			encodedAddresses.forEach(encoded => {
				expect(address.isValidEncodedAddress(encoded), encoded).to.equal(false);
			});
		});

		it('returns false for encoded address with wrong length', () => {
			// Arrange: added ABC
			const encoded = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFGABC';

			// Assert:
			expect(address.isValidEncodedAddress(encoded)).to.equal(false);
		});

		it('adding leading or trailing white space invalidates encoded address', () => {
			// Arrange:
			const encoded = 'NAR3W7B4BCOZSZMFIZRYB3N5YGOUSWIYJCJ6HDFG';

			// Assert:
			expect(address.isValidEncodedAddress(`   \t    ${encoded}`)).to.equal(false);
			expect(address.isValidEncodedAddress(`${encoded}   \t    `)).to.equal(false);
			expect(address.isValidEncodedAddress(`   \t    ${encoded}   \t    `)).to.equal(false);
		});
	});
});
