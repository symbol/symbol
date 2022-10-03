import { AesCbcCipher, AesGcmCipher } from '../src/Cipher.js';
import { SharedKey256 } from '../src/CryptoTypes.js';
import { hexToUint8 } from '../src/utils/converter.js';
import { expect } from 'chai';
import crypto from 'crypto';

describe('Cipher', () => {
	// region runBasicCipherTests

	const runBasicCipherTests = (CipherClass, testCases) => {
		testCases.forEach((testCase, index) => {
			it(`can encrypt, id: ${index}`, () => {
				// Arrange:
				const cipher = new CipherClass(testCase.sharedKey);

				// Act:
				const resultText = cipher.encrypt(testCase.clearText, testCase.iv);

				// Assert:
				expect(resultText).to.deep.equal(Uint8Array.of(...testCase.cipherText, ...testCase.tag));
			});

			it(`can decrypt, id: ${index}`, () => {
				// Arrange:
				const cipher = new CipherClass(testCase.sharedKey);

				// Act:
				const resultText = cipher.decrypt(Uint8Array.of(...testCase.cipherText, ...testCase.tag), testCase.iv);

				// Assert:
				expect(resultText).to.deep.equal(testCase.clearText);
			});
		});
	};

	// endregion

	// region AesCbcCipher

	describe('AesCbcCipher', () => {
		// test vectors taken from wycheproof project:
		// https://github.com/google/wycheproof/blob/master/testvectors/aes_cbc_pkcs5_test.json
		const testCases = [
			// tcId: 123
			{
				sharedKey: new SharedKey256('7BF9E536B66A215C22233FE2DAAA743A898B9ACB9F7802DE70B40E3D6E43EF97'),
				iv: hexToUint8('EB38EF61717E1324AE064E86F1C3E797'),
				tag: hexToUint8(''),
				cipherText: hexToUint8('E7C166554D1BB32792C981FA674CC4D8'),
				clearText: hexToUint8('')
			},
			// tcId: 127
			{
				sharedKey: new SharedKey256('E754076CEAB3FDAF4F9BCAB7D4F0DF0CBBAFBC87731B8F9B7CD2166472E8EEBC'),
				iv: hexToUint8('014D2E13DFBCB969BA3BB91442D52ECA'),
				tag: hexToUint8(''),
				cipherText: hexToUint8('42C0B89A706ED2606CD94F9CB361FA51'),
				clearText: hexToUint8('40')
			},
			// tcId: 125
			{
				sharedKey: new SharedKey256('96E1E4896FB2CD05F133A6A100BC5609A7AC3CA6D81721E922DADD69AD07A892'),
				iv: hexToUint8('E70D83A77A2CE722AC214C00837ACEDF'),
				tag: hexToUint8(''),
				cipherText: hexToUint8('A615A39FF8F59F82CF72ED13E1B01E32459700561BE112412961365C7A0B58AA7A16D68C065E77EBE504999051476BD7'),
				clearText: hexToUint8('91A17E4DFCC3166A1ADD26FF0E7C12056E8A654F28A6DE24F4BA739CEB5B5B18')
			}
		];
		runBasicCipherTests(AesCbcCipher, testCases);

		it('cannot decrypt with wrong iv', () => {
			// Arrange:
			const testCase = testCases[0];
			const cipher = new AesCbcCipher(testCase.sharedKey);

			// Act + Assert:
			expect(() => cipher.decrypt(testCase.cipherText, crypto.randomBytes(testCase.iv.length)))
				.to.throw(Error);
		});
	});

	// endregion

	// region AesGcmCipher

	describe('AesGcmCipher', () => {
		// test vectors taken from wycheproof project:
		// https://github.com/google/wycheproof/blob/master/testvectors/aes_gcm_test.json
		const testCases = [
			// tcId: 75
			{
				sharedKey: new SharedKey256('80BA3192C803CE965EA371D5FF073CF0F43B6A2AB576B208426E11409C09B9B0'),
				iv: hexToUint8('4DA5BF8DFD5852C1EA12379D'),
				tag: hexToUint8('4771A7C404A472966CEA8F73C8BFE17A'),
				cipherText: hexToUint8(''),
				clearText: hexToUint8('')
			},
			// tcId: 76
			{
				sharedKey: new SharedKey256('CC56B680552EB75008F5484B4CB803FA5063EBD6EAB91F6AB6AEF4916A766273'),
				iv: hexToUint8('99E23EC48985BCCDEEAB60F1'),
				tag: hexToUint8('633C1E9703EF744FFFFB40EDF9D14355'),
				cipherText: hexToUint8('06'),
				clearText: hexToUint8('2A')
			},
			// tcId: 87
			{
				sharedKey: new SharedKey256('D7ADDD3889FADF8C893EEE14BA2B7EA5BF56B449904869615BD05D5F114CF377'),
				iv: hexToUint8('8A3AD26B28CD13BA6504E260'),
				tag: hexToUint8('5E63374B519E6C3608321943D790CF9A'),
				cipherText: hexToUint8('53CC8C920A85D1ACCB88636D08BBE4869BFDD96F437B2EC944512173A9C0FE7A'
					+ '47F8434133989BA77DDA561B7E3701B9A83C3BA7660C666BA59FEF96598EB621'
					+ '544C63806D509AC47697412F9564EB0A2E1F72F6599F5666AF34CFFCA06573FF'
					+ 'B4F47B02F59F21C64363DAECB977B4415F19FDDA3C9AAE5066A57B669FFAA257'),
				clearText: hexToUint8('C877A76BF595560772167C6E3BCC705305DB9C6FCBEB90F4FEA85116038BC53C'
					+ '3FA5B4B4EA0DE5CC534FBE1CF9AE44824C6C2C0A5C885BD8C3CDC906F1267573'
					+ '7E434B983E1E231A52A275DB5FB1A0CAC6A07B3B7DCB19482A5D3B06A9317A54'
					+ '826CEA6B36FCE452FA9B5475E2AAF25499499D8A8932A19EB987C903BD8502FE')
			}
		];
		runBasicCipherTests(AesGcmCipher, testCases);

		it('cannot decrypt with wrong iv', () => {
			// Arrange:
			const testCase = testCases[0];
			const cipher = new AesGcmCipher(testCase.sharedKey);

			// Act + Assert:
			expect(() => cipher.decrypt(Uint8Array.of(...testCase.cipherText, ...testCase.tag), crypto.randomBytes(testCase.iv.length)))
				.to.throw(Error);
		});

		it('cannot decrypt with wrong tag', () => {
			// Arrange:
			const testCase = testCases[0];
			const cipher = new AesGcmCipher(testCase.sharedKey);

			// Act + Assert:
			expect(() => cipher.decrypt(Uint8Array.of(...testCase.cipherText, ...crypto.randomBytes(testCase.tag.length)), testCase.iv))
				.to.throw(Error);
		});
	});

	// endregion
});
