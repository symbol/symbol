const { AesCbcCipher, AesGcmCipher } = require('../src/Cipher');
const { PrivateKey, PublicKey, SharedKey256 } = require('../src/CryptoTypes');
const NemKeyPair = require('../src/nem/KeyPair').KeyPair;
const nemDeriveSharedKeyDeprecated = require('../src/nem/SharedKey').deriveSharedKeyDeprecated;
const SymbolKeyPair = require('../src/symbol/KeyPair').KeyPair;
const symbolDeriveSharedKey = require('../src/symbol/SharedKey').deriveSharedKey;
const { hexToUint8 } = require('../src/utils/converter');
const { expect } = require('chai');
const crypto = require('crypto');

describe('Cipher', () => {
	// region runBasicCipherTests

	const assertEncrypt = (cipher, testCase) => {
		// Act:
		const resultText = cipher.encrypt(testCase.clearText, testCase.iv);

		// Assert:
		expect(resultText).to.deep.equal(Uint8Array.of(...testCase.cipherText, ...testCase.tag));
	};

	const assertDecrypt = (cipher, testCase) => {
		// Act:
		const resultText = cipher.decrypt(Uint8Array.of(...testCase.cipherText, ...testCase.tag), testCase.iv);

		// Assert:
		expect(resultText).to.deep.equal(testCase.clearText);
	};

	const runBasicCipherTestsFull = testDescriptor => {
		it('can encrypt (full)', () => {
			// Arrange:
			const { testCase } = testDescriptor;
			const sharedKey = testDescriptor.deriveSharedKey(testCase);
			const cipher = new testDescriptor.CipherClass(sharedKey);

			// Act + Assert:
			assertEncrypt(cipher, testCase);
		});

		it('can decrypt (full)', () => {
			// Arrange:
			const { testCase } = testDescriptor;
			const sharedKey = testDescriptor.deriveSharedKey(testCase);
			const cipher = new testDescriptor.CipherClass(sharedKey);

			// Act + Assert:
			assertDecrypt(cipher, testCase);
		});
	};

	const runBasicCipherTestsSimple = (CipherClass, testCases) => {
		testCases.forEach((testCase, index) => {
			it(`can encrypt, id: ${index}`, () => {
				// Arrange:
				const cipher = new CipherClass(testCase.sharedKey);

				// Act + Assert:
				assertEncrypt(cipher, testCase);
			});

			it(`can decrypt, id: ${index}`, () => {
				// Arrange:
				const cipher = new CipherClass(testCase.sharedKey);

				// Act + Assert:
				assertDecrypt(cipher, testCase);
			});
		});
	};

	// endregion

	// region AesCbcCipher

	describe('AesCbcCipher', () => {
		const testDescriptor = {
			CipherClass: AesCbcCipher,
			testCase: {
				privateKey: new PrivateKey('3140F94C79F249787D1EC75A97A885980EB8F0A7D9B7AA03E7200296E422B2B6'),
				otherPublicKey: new PublicKey('9791ECCFA059DC3AAF27189A18B130FB592159BB8E87491640C30F0945B0433C'),
				salt: hexToUint8('AC6C43D4ECB3E9BC1753218D5C5AD0B91B053A2B7FFA3A0E0E65D0085FB38D79'),
				iv: hexToUint8('F15E192B633554AFDC2EDA91FC1EAD4E'),
				tag: new Uint8Array(),
				cipherText: hexToUint8('ECFB59733E063F2C2666E1FA77CE6449E8540B74F398395E9F06A683B72C246E'),
				clearText: hexToUint8('86DDB9E713A8EBF67A51830EFF03B837E147C20D75E67B2A54AA29E98C')
			},
			deriveSharedKey: testCaseDescriptor => nemDeriveSharedKeyDeprecated(
				new NemKeyPair(testCaseDescriptor.privateKey),
				testCaseDescriptor.otherPublicKey,
				testCaseDescriptor.salt
			)
		};
		runBasicCipherTestsFull(testDescriptor);

		// test vectors taken from wycheproof project:
		// https://github.com/google/wycheproof/blob/master/testvectors/aes_cbc_pkcs5_test.json
		runBasicCipherTestsSimple(AesCbcCipher, [
			// tcId: 123
			{
				sharedKey: new SharedKey256('7BF9E536B66A215C22233FE2DAAA743A898B9ACB9F7802DE70B40E3D6E43EF97'),
				iv: hexToUint8('EB38EF61717E1324AE064E86F1C3E797'),
				tag: hexToUint8(''),
				cipherText: hexToUint8('E7C166554D1BB32792C981FA674CC4D8'),
				clearText: hexToUint8('')
			},
			// tcId: 76
			{
				sharedKey: new SharedKey256('E754076CEAB3FDAF4F9BCAB7D4F0DF0CBBAFBC87731B8F9B7CD2166472E8EEBC'),
				iv: hexToUint8('014D2E13DFBCB969BA3BB91442D52ECA'),
				tag: hexToUint8(''),
				cipherText: hexToUint8('42C0B89A706ED2606CD94F9CB361FA51'),
				clearText: hexToUint8('40')
			},
			// tcId: 87
			{
				sharedKey: new SharedKey256('96E1E4896FB2CD05F133A6A100BC5609A7AC3CA6D81721E922DADD69AD07A892'),
				iv: hexToUint8('E70D83A77A2CE722AC214C00837ACEDF'),
				tag: hexToUint8(''),
				cipherText: hexToUint8('A615A39FF8F59F82CF72ED13E1B01E32459700561BE112412961365C7A0B58AA7A16D68C065E77EBE504999051476BD7'),
				clearText: hexToUint8('91A17E4DFCC3166A1ADD26FF0E7C12056E8A654F28A6DE24F4BA739CEB5B5B18')
			}
		]);

		it('cannot decrypt with wrong iv', () => {
			// Arrange:
			const { testCase } = testDescriptor;
			const sharedKey = testDescriptor.deriveSharedKey(testCase);

			const cipher = new testDescriptor.CipherClass(sharedKey);

			// Act:
			const resultText = cipher.decrypt(testCase.cipherText, crypto.randomBytes(testCase.iv.length));

			// Assert:
			expect(resultText).to.not.deep.equal(testCase.clearText);
		});
	});

	// endregion

	// region AesGcmCipher

	describe('AesGcmCipher', () => {
		const testDescriptor = {
			CipherClass: AesGcmCipher,
			testCase: {
				privateKey: new PrivateKey('3140F94C79F249787D1EC75A97A885980EB8F0A7D9B7AA03E7200296E422B2B6'),
				otherPublicKey: new PublicKey('C62827148875ACAF05D25D29B1BB1D947396A89CE41CB48888AE6961D9991DDF'),
				iv: hexToUint8('A73FF5C32F8FD055B0977581'),
				tag: hexToUint8('1B3370262014CB148F7A8CC88D344370'),
				cipherText: hexToUint8('1B1398B84750C9DDEE99164AA1A54C89E9705FDCEBACD05A7B75F1E716'),
				clearText: hexToUint8('86DDB9E713A8EBF67A51830EFF03B837E147C20D75E67B2A54AA29E98C')
			},
			deriveSharedKey: testCaseDescriptor => symbolDeriveSharedKey(
				new SymbolKeyPair(testCaseDescriptor.privateKey),
				testCaseDescriptor.otherPublicKey
			)
		};
		runBasicCipherTestsFull(testDescriptor);

		// test vectors taken from wycheproof project:
		// https://github.com/google/wycheproof/blob/master/testvectors/aes_gcm_test.json
		runBasicCipherTestsSimple(AesGcmCipher, [
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
		]);

		it('cannot decrypt with wrong iv', () => {
			// Arrange:
			const { testCase } = testDescriptor;
			const sharedKey = testDescriptor.deriveSharedKey(testCase);

			const cipher = new testDescriptor.CipherClass(sharedKey);

			// Act + Assert:
			expect(() => cipher.decrypt(Uint8Array.of(...testCase.cipherText, ...testCase.tag), crypto.randomBytes(testCase.iv.length)))
				.to.throw(Error);
		});

		it('cannot decrypt with wrong tag', () => {
			// Arrange:
			const { testCase } = testDescriptor;
			const sharedKey = testDescriptor.deriveSharedKey(testCase);

			const cipher = new testDescriptor.CipherClass(sharedKey);

			// Act + Assert:
			expect(() => cipher.decrypt(Uint8Array.of(...testCase.cipherText, ...crypto.randomBytes(testCase.tag.length)), testCase.iv))
				.to.throw(Error);
		});
	});

	// endregion
});
