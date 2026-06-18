package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.utils.Converter;

final class CipherTypesTest {
	private record TestCase(String sharedKeyHex, String ivHex, String tagHex, String cipherTextHex, String clearTextHex) {
		CryptoTypes.SharedKey256 sharedKey() {
			return new CryptoTypes.SharedKey256(sharedKeyHex);
		}

		byte[] iv() {
			return Converter.hexToUint8(ivHex);
		}

		byte[] tag() {
			return Converter.hexToUint8(tagHex);
		}

		byte[] cipherText() {
			return Converter.hexToUint8(cipherTextHex);
		}

		byte[] clearText() {
			return Converter.hexToUint8(clearTextHex);
		}

		byte[] cipherTextWithTag() {
			final byte[] ct = cipherText();
			final byte[] tg = tag();
			final byte[] out = new byte[ct.length + tg.length];
			System.arraycopy(ct, 0, out, 0, ct.length);
			System.arraycopy(tg, 0, out, ct.length, tg.length);
			return out;
		}
	}

	@Nested
	final class AesCbcCipherTest {
		// Test vectors from the wycheproof project (aes_cbc_pkcs5_test.json).
		private final TestCase[] testCases = {
				new TestCase("7BF9E536B66A215C22233FE2DAAA743A898B9ACB9F7802DE70B40E3D6E43EF97", "EB38EF61717E1324AE064E86F1C3E797", "",
						"E7C166554D1BB32792C981FA674CC4D8", ""),
				new TestCase("E754076CEAB3FDAF4F9BCAB7D4F0DF0CBBAFBC87731B8F9B7CD2166472E8EEBC", "014D2E13DFBCB969BA3BB91442D52ECA", "",
						"42C0B89A706ED2606CD94F9CB361FA51", "40"),
				new TestCase("96E1E4896FB2CD05F133A6A100BC5609A7AC3CA6D81721E922DADD69AD07A892", "E70D83A77A2CE722AC214C00837ACEDF", "",
						"A615A39FF8F59F82CF72ED13E1B01E32459700561BE112412961365C7A0B58AA7A16D68C065E77EBE504999051476BD7",
						"91A17E4DFCC3166A1ADD26FF0E7C12056E8A654F28A6DE24F4BA739CEB5B5B18")
		};

		@Test
		void canEncrypt() {
			// Act + Assert:
			for (int i = 0; i < testCases.length; ++i) {
				final TestCase tc = testCases[i];
				final CipherTypes.AesCbcCipher cipher = new CipherTypes.AesCbcCipher(tc.sharedKey());
				final byte[] result = cipher.encrypt(tc.clearText(), tc.iv());
				assertThat("id " + i, result, equalTo(tc.cipherTextWithTag()));
			}
		}

		@Test
		void canDecrypt() {
			// Act + Assert:
			for (int i = 0; i < testCases.length; ++i) {
				final TestCase tc = testCases[i];
				final CipherTypes.AesCbcCipher cipher = new CipherTypes.AesCbcCipher(tc.sharedKey());
				final byte[] result = cipher.decrypt(tc.cipherTextWithTag(), tc.iv());
				assertThat("id " + i, result, equalTo(tc.clearText()));
			}
		}

		@Test
		void cannotDecryptWithWrongIv() {
			// Arrange:
			final TestCase tc = testCases[0];
			final CipherTypes.AesCbcCipher cipher = new CipherTypes.AesCbcCipher(tc.sharedKey());
			final byte[] wrongIv = new byte[tc.iv().length];
			new java.security.SecureRandom().nextBytes(wrongIv);

			// Act + Assert:
			assertThrows(IllegalStateException.class, () -> cipher.decrypt(tc.cipherText(), wrongIv));
		}

		@Test
		void encryptWrapsSecurityExceptionAsIllegalState() {
			// Arrange:
			final CipherTypes.AesCbcCipher cipher = new CipherTypes.AesCbcCipher(testCases[0].sharedKey());

			// Act + Assert:
			// AES/CBC requires a 16-byte IV; a 3-byte IV makes Cipher.init throw a GeneralSecurityException.
			assertThrows(IllegalStateException.class, () -> cipher.encrypt(new byte[16], new byte[3]));
		}
	}

	@Nested
	final class AesGcmCipherTest {
		// Test vectors from the wycheproof project (aes_gcm_test.json).
		private final TestCase[] testCases = {
				new TestCase("80BA3192C803CE965EA371D5FF073CF0F43B6A2AB576B208426E11409C09B9B0", "4DA5BF8DFD5852C1EA12379D",
						"4771A7C404A472966CEA8F73C8BFE17A", "", ""),
				new TestCase("CC56B680552EB75008F5484B4CB803FA5063EBD6EAB91F6AB6AEF4916A766273", "99E23EC48985BCCDEEAB60F1",
						"633C1E9703EF744FFFFB40EDF9D14355", "06", "2A"),
				new TestCase("D7ADDD3889FADF8C893EEE14BA2B7EA5BF56B449904869615BD05D5F114CF377", "8A3AD26B28CD13BA6504E260",
						"5E63374B519E6C3608321943D790CF9A",
						"53CC8C920A85D1ACCB88636D08BBE4869BFDD96F437B2EC944512173A9C0FE7A"
								+ "47F8434133989BA77DDA561B7E3701B9A83C3BA7660C666BA59FEF96598EB621"
								+ "544C63806D509AC47697412F9564EB0A2E1F72F6599F5666AF34CFFCA06573FF"
								+ "B4F47B02F59F21C64363DAECB977B4415F19FDDA3C9AAE5066A57B669FFAA257",
						"C877A76BF595560772167C6E3BCC705305DB9C6FCBEB90F4FEA85116038BC53C"
								+ "3FA5B4B4EA0DE5CC534FBE1CF9AE44824C6C2C0A5C885BD8C3CDC906F1267573"
								+ "7E434B983E1E231A52A275DB5FB1A0CAC6A07B3B7DCB19482A5D3B06A9317A54"
								+ "826CEA6B36FCE452FA9B5475E2AAF25499499D8A8932A19EB987C903BD8502FE")
		};

		@Test
		void canEncrypt() {
			// Act + Assert:
			for (int i = 0; i < testCases.length; ++i) {
				final TestCase tc = testCases[i];
				final CipherTypes.AesGcmCipher cipher = new CipherTypes.AesGcmCipher(tc.sharedKey());
				final byte[] result = cipher.encrypt(tc.clearText(), tc.iv());
				assertThat("id " + i, result, equalTo(tc.cipherTextWithTag()));
			}
		}

		@Test
		void canDecrypt() {
			// Act + Assert:
			for (int i = 0; i < testCases.length; ++i) {
				final TestCase tc = testCases[i];
				final CipherTypes.AesGcmCipher cipher = new CipherTypes.AesGcmCipher(tc.sharedKey());
				final byte[] result = cipher.decrypt(tc.cipherTextWithTag(), tc.iv());
				assertThat("id " + i, result, equalTo(tc.clearText()));
			}
		}

		@Test
		void cannotDecryptWithWrongIv() {
			// Arrange:
			final TestCase tc = testCases[0];
			final CipherTypes.AesGcmCipher cipher = new CipherTypes.AesGcmCipher(tc.sharedKey());
			final byte[] wrongIv = new byte[tc.iv().length];
			new java.security.SecureRandom().nextBytes(wrongIv);

			// Act + Assert:
			assertThrows(IllegalStateException.class, () -> cipher.decrypt(tc.cipherTextWithTag(), wrongIv));
		}

		@Test
		void cannotDecryptWithWrongTag() {
			// Arrange:
			final TestCase tc = testCases[0];
			final CipherTypes.AesGcmCipher cipher = new CipherTypes.AesGcmCipher(tc.sharedKey());
			final byte[] tagBytes = new byte[tc.tag().length];
			new java.security.SecureRandom().nextBytes(tagBytes);
			final byte[] cipherText = tc.cipherText();
			final byte[] withWrongTag = new byte[cipherText.length + tagBytes.length];
			System.arraycopy(cipherText, 0, withWrongTag, 0, cipherText.length);
			System.arraycopy(tagBytes, 0, withWrongTag, cipherText.length, tagBytes.length);

			// Act + Assert:
			assertThrows(IllegalStateException.class, () -> cipher.decrypt(withWrongTag, tc.iv()));
		}
	}
}
