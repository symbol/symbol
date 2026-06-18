package org.symbol.sdk;

import javax.crypto.Cipher;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

/**
 * AES symmetric ciphers: {@link AesCbcCipher} (AES-256-CBC with PKCS#7 padding) and {@link AesGcmCipher} (AES-256-GCM with the
 * authentication tag appended to the ciphertext).
 */
public final class CipherTypes {
	private CipherTypes() {
	}

	private static final String AES = "AES";

	/**
	 * Performs AES CBC encryption and decryption with a given key.
	 */
	public static final class AesCbcCipher {
		private static final String TRANSFORMATION = "AES/CBC/PKCS5Padding";

		private final SecretKeySpec key;

		/**
		 * Creates a cipher around an AES shared key.
		 *
		 * @param aesKey AES shared key.
		 */
		public AesCbcCipher(final CryptoTypes.SharedKey256 aesKey) {
			this.key = new SecretKeySpec(aesKey.bytes(), AES);
		}

		/**
		 * Encrypts clear text.
		 *
		 * @param clearText Clear text to encrypt.
		 * @param iv IV bytes.
		 * @return Cipher text.
		 */
		public byte[] encrypt(final byte[] clearText, final byte[] iv) {
			try {
				final Cipher cipher = Cipher.getInstance(TRANSFORMATION);
				cipher.init(Cipher.ENCRYPT_MODE, key, new IvParameterSpec(iv));
				return cipher.doFinal(clearText);
			} catch (java.security.GeneralSecurityException ex) {
				throw new IllegalStateException(ex);
			}
		}

		/**
		 * Decrypts cipher text.
		 *
		 * @param cipherText Cipher text to decrypt.
		 * @param iv IV bytes.
		 * @return Clear text.
		 */
		public byte[] decrypt(final byte[] cipherText, final byte[] iv) {
			try {
				final Cipher cipher = Cipher.getInstance(TRANSFORMATION);
				cipher.init(Cipher.DECRYPT_MODE, key, new IvParameterSpec(iv));
				return cipher.doFinal(cipherText);
			} catch (java.security.GeneralSecurityException ex) {
				throw new IllegalStateException(ex);
			}
		}
	}

	/**
	 * Performs AES GCM encryption and decryption with a given key.
	 */
	public static final class AesGcmCipher {
		/**
		 * Byte size of GCM tag.
		 */
		public static final int TAG_SIZE = 16;

		private static final String TRANSFORMATION = "AES/GCM/NoPadding";

		private final SecretKeySpec key;

		/**
		 * Creates a cipher around an AES shared key.
		 *
		 * @param aesKey AES shared key.
		 */
		public AesGcmCipher(final CryptoTypes.SharedKey256 aesKey) {
			this.key = new SecretKeySpec(aesKey.bytes(), AES);
		}

		/**
		 * Encrypts clear text and appends the authentication tag to the encrypted payload.
		 *
		 * @param clearText Clear text to encrypt.
		 * @param iv IV bytes.
		 * @return Cipher text with appended tag.
		 */
		public byte[] encrypt(final byte[] clearText, final byte[] iv) {
			try {
				final Cipher cipher = Cipher.getInstance(TRANSFORMATION);
				cipher.init(Cipher.ENCRYPT_MODE, key, new GCMParameterSpec(TAG_SIZE * 8, iv));
				// JDK's GCM cipher already appends the auth tag at the end of the output of doFinal.
				return cipher.doFinal(clearText);
			} catch (java.security.GeneralSecurityException ex) {
				throw new IllegalStateException(ex);
			}
		}

		/**
		 * Decrypts cipher text with appended tag.
		 *
		 * @param cipherText Cipher text with appended tag to decrypt.
		 * @param iv IV bytes.
		 * @return Clear text.
		 */
		public byte[] decrypt(final byte[] cipherText, final byte[] iv) {
			try {
				final Cipher cipher = Cipher.getInstance(TRANSFORMATION);
				cipher.init(Cipher.DECRYPT_MODE, key, new GCMParameterSpec(TAG_SIZE * 8, iv));
				return cipher.doFinal(cipherText);
			} catch (java.security.GeneralSecurityException ex) {
				throw new IllegalStateException(ex);
			}
		}
	}
}
