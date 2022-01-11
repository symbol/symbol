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

const { createKeyPairFromPrivateKeyString, sign, verify } = require('../../src/crypto/keyPair');
const convert = require('../../src/utils/convert');
const test = require('../testUtils');
const { expect } = require('chai');

describe('key pair', () => {
	const Private_Key_Size = 32;
	const Signature_Size = 64;

	const Private_Keys = [
		'ABF4CF55A2B3F742D7543D9CC17F50447B969E6E06F5EA9195D428AB12B7318D',
		'6AA6DAD25D3ACB3385D5643293133936CDDDD7F7E11818771DB1FF2F9D3F9215',
		'8E32BC030A4C53DE782EC75BA7D5E25E64A2A072A56E5170B77A4924EF3C32A9',
		'C83CE30FCB5B81A51BA58FF827CCBC0142D61C13E2ED39E78E876605DA16D8D7',
		'2DA2A0AAE0F37235957B51D15843EDDE348A559692D8FA87B94848459899FC27'
	];

	describe('construction', () => {
		it('can extract from private key test vectors', () => {
			// Arrange:
			const Expected_Public_Keys = [
				'4DB881D07086498C3626F1F84EF89D7E08E5D8293298400F27CA98C92AB2D271',
				'F7BBE3BB4DBF9698122DA02EB8A6EDE55F1EF90D0C64819E8A792231A2A0B143',
				'41C7467803C694DC7CB1D11384AD35BF63873E21EC04454E434FE64934942621',
				'4CD65AE31B90557EA0F80BCA0748AE1C91C9A1FB53666E8DCCC176774B94E52A',
				'37C877158F0BCCEF264475AF113494A0A385CB01CDA2ABCEC93C76A8EFC537A8'
			];

			// Sanity:
			expect(Private_Keys.length).equal(Expected_Public_Keys.length);

			for (let i = 0; i < Private_Keys.length; ++i) {
				// Arrange:
				const privateKeyHex = Private_Keys[i];
				const expectedPublicKey = Expected_Public_Keys[i];

				// Act:
				const keyPair = createKeyPairFromPrivateKeyString(privateKeyHex);

				// Assert:
				const message = ` from ${privateKeyHex}`;
				expect(convert.uint8ToHex(keyPair.publicKey), `public ${message}`).equal(expectedPublicKey);
				expect(convert.uint8ToHex(keyPair.privateKey), `private ${message}`).equal(privateKeyHex);
			}
		});

		it('cannot extract from invalid private key', () => {
			// Arrange:
			const invalidPrivateKeys = [
				'', // empty
				'53C659B47C176A70EB228DE5C0A0FF391282C96640C2A42CD5BBD0982176AB', // short
				'53C659B47C176A70EB228DE5C0A0FF391282C96640C2A42CD5BBD0982176AB1BBB' // long
			];

			// Act:
			invalidPrivateKeys.forEach(privateKey => {
				// Assert:
				expect(() => { createKeyPairFromPrivateKeyString(privateKey); }, `from ${privateKey}`)
					.to.throw('private key has unexpected size');
			});
		});
	});

	describe('sign', () => {
		it('fills the signature', () => {
			// Arrange:
			const keyPair = test.random.keyPair();
			const payload = test.random.bytes(100);

			// Act:
			const signature = sign(keyPair, payload);

			// Assert:
			expect(signature).to.not.deep.equal(new Uint8Array(Signature_Size));
		});

		it('returns same signature for same data signed by same key pairs', () => {
			// Arrange:
			const privateKey = convert.uint8ToHex(test.random.bytes(Private_Key_Size));
			const keyPair1 = createKeyPairFromPrivateKeyString(privateKey);
			const keyPair2 = createKeyPairFromPrivateKeyString(privateKey);
			const payload = test.random.bytes(100);

			// Act:
			const signature1 = sign(keyPair1, payload);
			const signature2 = sign(keyPair2, payload);

			// Assert:
			expect(signature2).to.deep.equal(signature1);
		});

		it('returns different signature for same data signed by different key pairs', () => {
			// Arrange:
			const keyPair1 = test.random.keyPair();
			const keyPair2 = test.random.keyPair();
			const payload = test.random.bytes(100);

			// Act:
			const signature1 = sign(keyPair1, payload);
			const signature2 = sign(keyPair2, payload);

			// Assert:
			expect(signature2).to.not.deep.equal(signature1);
		});

		it('cannot sign unsupported data type', () => {
			// Arrange:
			const keyPair = createKeyPairFromPrivateKeyString(Private_Keys[0]);

			// Assert:
			expect(() => { sign(keyPair, {}); }).to.throw('unexpected type, use Uint8Array');
		});
	});

	describe('verify', () => {
		it('returns true for data signed with same key pair', () => {
			// Arrange:
			const keyPair = test.random.keyPair();
			const payload = test.random.bytes(100);
			const signature = sign(keyPair, payload);

			// Act:
			const isVerified = verify(keyPair.publicKey, payload, signature);

			// Assert:
			expect(isVerified).to.equal(true);
		});

		it('returns false for data signed with different key pair', () => {
			// Arrange:
			const keyPair1 = test.random.keyPair();
			const keyPair2 = test.random.keyPair();
			const payload = test.random.bytes(100);
			const signature = sign(keyPair1, payload);

			// Act:
			const isVerified = verify(keyPair2.publicKey, payload, signature);

			// Assert:
			expect(isVerified).to.equal(false);
		});

		it('returns false if signature has been modified', () => {
			// Arrange:
			const keyPair = test.random.keyPair();
			const payload = test.random.bytes(100);

			for (let i = 0; i < Signature_Size; i += 4) {
				const signature = sign(keyPair, payload);
				signature[i] ^= 0xFF;

				// Act:
				const isVerified = verify(keyPair.publicKey, payload, signature);

				// Assert:
				expect(isVerified, `signature modified at ${i}`).to.equal(false);
			}
		});

		it('returns false if payload has been modified', () => {
			// Arrange:
			const keyPair = test.random.keyPair();
			const payload = test.random.bytes(44);

			for (let i = 0; i < payload.length; i += 4) {
				const signature = sign(keyPair, payload);
				payload[i] ^= 0xFF;

				// Act:
				const isVerified = verify(keyPair.publicKey, payload, signature);

				// Assert:
				expect(isVerified, `payload modified at ${i}`).to.equal(false);
			}
		});

		it('fails if public key is not on curve', () => {
			// Arrange:
			const keyPair = test.random.keyPair();
			keyPair.publicKey.fill(0);
			keyPair.publicKey[keyPair.publicKey.length - 1] = 1;

			const payload = test.random.bytes(100);
			const signature = sign(keyPair, payload);

			// Act:
			const isVerified = verify(keyPair.publicKey, payload, signature);

			// Assert:
			expect(isVerified).to.equal(false);
		});

		it('fails if public key does not correspond to private key', () => {
			// Arrange:
			const keyPair = test.random.keyPair();
			const payload = test.random.bytes(100);
			const signature = sign(keyPair, payload);

			for (let i = 0; i < keyPair.publicKey.length; ++i)
				keyPair.publicKey[i] ^= 0xFF;

			// Act:
			const isVerified = verify(keyPair.publicKey, payload, signature);

			// Assert:
			expect(isVerified).to.equal(false);
		});

		it('rejects zero public key', () => {
			// Arrange:
			const keyPair = test.random.keyPair();
			keyPair.publicKey.fill(0);

			const payload = test.random.bytes(100);
			const signature = sign(keyPair, payload);

			// Act:
			const isVerified = verify(keyPair.publicKey, payload, signature);

			// Assert:
			expect(isVerified).to.equal(false);
		});

		it('cannot verify non canonical signature', () => {
			const scalarAddGroupOrder = scalar => {
				// 2^252 + 27742317777372353535851937790883648493, little endian
				const Group_Order = [
					0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10
				];

				let r = 0;
				for (let i = 0; i < scalar.length; ++i) {
					const t = scalar[i] + Group_Order[i];
					scalar[i] += Group_Order[i] + r;
					r = (t >> 8) & 0xFF;
				}
			};

			// Arrange:
			const keyPair = test.random.keyPair();
			const payload = new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 0]);
			const canonicalSignature = sign(keyPair, payload);

			// this is signature with group order added to 'encodedS' part of signature
			const nonCanonicalSignature = canonicalSignature.slice();
			scalarAddGroupOrder(nonCanonicalSignature.subarray(32));

			// Act:
			const isCanonicalVerified = verify(keyPair.publicKey, payload, canonicalSignature);
			const isNonCanonicalVerified = verify(keyPair.privateKey, payload, nonCanonicalSignature);

			// Assert:
			expect(isCanonicalVerified).to.equal(true);
			expect(isNonCanonicalVerified).to.equal(false);
		});
	});

	describe('test vectors', () => {
		const Input_Data = [
			'8ce03cd60514233b86789729102ea09e867fc6d964dea8c2018ef7d0a2e0e24bf7e348e917116690b9',
			'e4a92208a6fc52282b620699191ee6fb9cf04daf48b48fd542c5e43daa9897763a199aaa4b6f10546109f47ac3564fade0',
			'13ed795344c4448a3b256f23665336645a853c5c44dbff6db1b9224b5303b6447fbf8240a2249c55',
			'a2704638434e9f7340f22d08019c4c8e3dbee0df8dd4454a1d70844de11694f4c8ca67fdcb08fed0cec9abb2112b5e5f89',
			'd2488e854dbcdfdb2c9d16c8c0b2fdbc0abb6bac991bfe2b14d359a6bc99d66c00fd60d731ae06d0'
		];

		const Expected_Signatures = [
			/* eslint-disable max-len */
			'31D272F0662915CAC43AB7D721CAF65D8601F52B2E793EA1533E7BC20E04EA97B74859D9209A7B18DFECFD2C4A42D6957628F5357E3FB8B87CF6A888BAB4280E',
			'F21E4BE0A914C0C023F724E1EAB9071A3743887BB8824CB170404475873A827B301464261E93700725E8D4427A3E39D365AFB2C9191F75D33C6BE55896E0CC00',
			'939CD8932093571E24B21EA53F1359279BA5CFC32CE99BB020E676CF82B0AA1DD4BC76FCDE41EF784C06D122B3D018135352C057F079C926B3EFFA7E73CF1D06',
			'9B4AFBB7B96CAD7726389C2A4F31115940E6EEE3EA29B3293C82EC8C03B9555C183ED1C55CA89A58C17729EFBA76A505C79AA40EC618D83124BC1134B887D305',
			'7AF2F0D9B30DE3B6C40605FDD4EBA93ECE39FA7458B300D538EC8D0ABAC1756DEFC0CA84C8A599954313E58CE36EFBA4C24A82FD6BB8127023A58EFC52A8410A'
			/* eslint-enable max-len */
		];

		it('can sign test vectors as binary', () => {
			// Sanity:
			expect(Private_Keys.length).equal(Input_Data.length);
			expect(Private_Keys.length).equal(Expected_Signatures.length);

			for (let i = 0; i < Private_Keys.length; ++i) {
				// Arrange:
				const inputData = convert.hexToUint8(Input_Data[i]);
				const keyPair = createKeyPairFromPrivateKeyString(Private_Keys[i]);

				// Act:
				const signature = sign(keyPair, inputData);

				// Assert:
				const message = `signing with ${Private_Keys[i]}`;
				expect(convert.uint8ToHex(signature), message).equal(Expected_Signatures[i]);
			}
		});

		it('can verify test vectors as binary', () => {
			// Sanity:
			expect(Private_Keys.length).equal(Input_Data.length);
			expect(Private_Keys.length).equal(Expected_Signatures.length);

			for (let i = 0; i < Private_Keys.length; ++i) {
				// Arrange:
				const inputData = convert.hexToUint8(Input_Data[i]);
				const keyPair = createKeyPairFromPrivateKeyString(Private_Keys[i]);
				const signature = sign(keyPair, inputData);

				// Act:
				const isVerified = verify(keyPair.publicKey, inputData, signature);

				// Assert:
				const message = `verifying with ${Private_Keys[i]}`;
				expect(isVerified, message).equal(true);
			}
		});
	});
});
