package org.symbol.sdk.impl;

import java.security.InvalidAlgorithmParameterException;
import java.security.SecureRandom;
import java.util.Arrays;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;

import org.symbol.sdk.CipherTypes.AesCbcCipher;
import org.symbol.sdk.CipherTypes.AesGcmCipher;
import org.symbol.sdk.CryptoException;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.utils.ArrayHelpers;

/**
 * Cipher plumbing shared by the NEM and Symbol message encoders.
 */
public final class CipherHelpers {

	/** AES-GCM initialization vector size. */
	public static final int GCM_IV_SIZE = 12;

	/** AES-CBC initialization vector size. */
	public static final int CBC_IV_SIZE = 16;

	private static final int TAG_SIZE = AesGcmCipher.TAG_SIZE;

	// shared, thread-safe SecureRandom; per-call allocation reseeds the entropy source each time (mirrors CryptoTypes)
	private static final SecureRandom SECURE_RANDOM = new SecureRandom();

	private CipherHelpers() {
	}

	/**
	 * Result of an AES-GCM encode.
	 *
	 * @param tag Authentication tag.
	 * @param initializationVector Initialization vector.
	 * @param cipherText Cipher text.
	 */
	public record EncodedGcm(byte[] tag, byte[] initializationVector, byte[] cipherText) {
	}

	/**
	 * Result of an AES-CBC encode.
	 *
	 * @param initializationVector Initialization vector.
	 * @param cipherText Cipher text.
	 */
	public record EncodedCbc(byte[] initializationVector, byte[] cipherText) {
	}

	/**
	 * Encrypts a message with AES-GCM under a random initialization vector.
	 *
	 * @param sharedKey Shared encryption key.
	 * @param message Message to encrypt.
	 * @return Tag, initialization vector and cipher text.
	 */
	public static EncodedGcm encodeAesGcm(final CryptoTypes.SharedKey256 sharedKey, final byte[] message) {
		final AesGcmCipher cipher = new AesGcmCipher(sharedKey);
		final byte[] iv = new byte[GCM_IV_SIZE];
		SECURE_RANDOM.nextBytes(iv);
		final byte[] cipherTextPlusTag = cipher.encrypt(message, iv);
		final int tagOffset = cipherTextPlusTag.length - TAG_SIZE;
		final byte[] cipherText = Arrays.copyOfRange(cipherTextPlusTag, 0, tagOffset);
		final byte[] tag = Arrays.copyOfRange(cipherTextPlusTag, tagOffset, cipherTextPlusTag.length);
		return new EncodedGcm(tag, iv, cipherText);
	}

	/**
	 * Tries to decrypt an AES-GCM payload laid out as {@code [tag | iv | cipherText]}.
	 *
	 * @param sharedKey Shared encryption key.
	 * @param tagIvCipher Encoded payload.
	 * @return Decrypted bytes, or {@code null} when the payload is too short or fails AEAD authentication (the message was not for this
	 *         key); any other cipher failure (an environment fault) propagates.
	 */
	public static byte[] tryDecodeAesGcm(final CryptoTypes.SharedKey256 sharedKey, final byte[] tagIvCipher) {
		if (tagIvCipher.length < TAG_SIZE + GCM_IV_SIZE)
			return null;

		final byte[] tag = Arrays.copyOfRange(tagIvCipher, 0, TAG_SIZE);
		final byte[] iv = Arrays.copyOfRange(tagIvCipher, TAG_SIZE, TAG_SIZE + GCM_IV_SIZE);
		final byte[] cipherText = Arrays.copyOfRange(tagIvCipher, TAG_SIZE + GCM_IV_SIZE, tagIvCipher.length);

		final AesGcmCipher cipher = new AesGcmCipher(sharedKey);
		try {
			// JDK's GCM expects the auth tag appended to the ciphertext
			return cipher.decrypt(ArrayHelpers.concat(cipherText, tag), iv);
		} catch (final CryptoException ex) {
			if (ex.getCause() instanceof BadPaddingException)
				return null;

			throw ex;
		}
	}

	/**
	 * Encrypts a message with AES-CBC under a random initialization vector.
	 *
	 * @param sharedKey Shared encryption key.
	 * @param message Message to encrypt.
	 * @return Initialization vector and cipher text.
	 */
	public static EncodedCbc encodeAesCbc(final CryptoTypes.SharedKey256 sharedKey, final byte[] message) {
		final AesCbcCipher cipher = new AesCbcCipher(sharedKey);
		final byte[] iv = new byte[CBC_IV_SIZE];
		SECURE_RANDOM.nextBytes(iv);
		return new EncodedCbc(iv, cipher.encrypt(message, iv));
	}

	/**
	 * Tries to decrypt an AES-CBC payload laid out as {@code [iv | cipherText]}.
	 *
	 * @param sharedKey Shared encryption key.
	 * @param ivCipher Encoded payload.
	 * @return Decrypted bytes, or {@code null} when the payload is too short or fails with a padding / block-length / IV error (a wrong
	 *         key); any other cipher failure (an environment fault) propagates.
	 */
	public static byte[] tryDecodeAesCbc(final CryptoTypes.SharedKey256 sharedKey, final byte[] ivCipher) {
		if (ivCipher.length < CBC_IV_SIZE)
			return null;

		final byte[] iv = Arrays.copyOfRange(ivCipher, 0, CBC_IV_SIZE);
		final byte[] cipherText = Arrays.copyOfRange(ivCipher, CBC_IV_SIZE, ivCipher.length);

		final AesCbcCipher cipher = new AesCbcCipher(sharedKey);
		try {
			return cipher.decrypt(cipherText, iv);
		} catch (final CryptoException ex) {
			final Throwable cause = ex.getCause();
			if (cause instanceof BadPaddingException || cause instanceof IllegalBlockSizeException
					|| cause instanceof InvalidAlgorithmParameterException)
				return null;

			throw ex;
		}
	}
}
