package org.symbol.sdk.nem;

import java.security.SecureRandom;
import java.util.Arrays;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.MessageEncoderResult;
import org.symbol.sdk.impl.CipherHelpers;
import org.symbol.sdk.nem.models.*;
import org.symbol.sdk.utils.ArrayHelpers;

/**
 * Encrypts / encodes messages between two NEM parties; {@code tryDecode} tries AES-GCM ({@code [tag(16) | iv(12) | cipherText]}) then falls
 * back to deprecated AES-CBC ({@code [salt(32) | iv(16) | cipherText]}, the salt randomising the deprecated shared-key derivation).
 */
public final class MessageEncoder {

	private static final int SALT_SIZE = 32;

	// shared, thread-safe SecureRandom; per-call allocation reseeds the entropy source each time (mirrors CryptoTypes)
	private static final SecureRandom SECURE_RANDOM = new SecureRandom();

	private final KeyPair keyPair;

	/**
	 * Creates a message encoder around a key pair.
	 *
	 * @param keyPair Key pair.
	 */
	public MessageEncoder(final KeyPair keyPair) {
		this.keyPair = keyPair;
	}

	/**
	 * Returns the public key used for message encoding.
	 *
	 * @return Public key used for message encoding.
	 */
	public CryptoTypes.PublicKey getPublicKey() {
		return keyPair.getPublicKey();
	}

	/**
	 * Tries to decode an encoded message.
	 *
	 * @param recipientPublicKey Recipient public key.
	 * @param encodedMessage Encoded message.
	 * @return Decoded result; falls back to the original message when both GCM and CBC fail.
	 * @throws IllegalArgumentException When {@code encodedMessage.messageType} is not {@link MessageType#ENCRYPTED}, or when a non-GCM
	 *             payload is shorter than the CBC salt.
	 */
	public MessageEncoderResult tryDecode(final CryptoTypes.PublicKey recipientPublicKey, final Message encodedMessage) {
		if (MessageType.ENCRYPTED != encodedMessage.getMessageType())
			throw new IllegalArgumentException("invalid message format");

		final byte[] payload = encodedMessage.getMessage();

		byte[] decoded = tryDecodeGcm(recipientPublicKey, payload);
		if (null != decoded)
			return new MessageEncoderResult(true, decoded);

		decoded = tryDecodeCbc(recipientPublicKey, payload);
		if (null != decoded)
			return new MessageEncoderResult(true, decoded);

		return new MessageEncoderResult(false, encodedMessage);
	}

	/**
	 * Encodes a message to a recipient using the recommended format.
	 *
	 * @param recipientPublicKey Recipient public key.
	 * @param message Message to encode.
	 * @return Encrypted and encoded message.
	 */
	public Message encode(final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
		final CipherHelpers.EncodedGcm encoded = CipherHelpers.encodeAesGcm(SharedKey.deriveSharedKey(this.keyPair, recipientPublicKey),
				message);

		final Message encodedMessage = new Message();
		encodedMessage.setMessageType(MessageType.ENCRYPTED);
		encodedMessage.setMessage(ArrayHelpers.concat(encoded.tag(), encoded.initializationVector(), encoded.cipherText()));
		return encodedMessage;
	}

	/**
	 * Encodes a message using the deprecated AES-CBC format; only for compatibility with older NEM messages — new code should use
	 * {@link #encode(CryptoTypes.PublicKey, byte[])}.
	 *
	 * @param recipientPublicKey Recipient public key.
	 * @param message Message to encode.
	 * @return Encrypted and encoded message.
	 */
	public Message encodeDeprecated(final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
		final byte[] salt = new byte[SALT_SIZE];
		SECURE_RANDOM.nextBytes(salt);
		final CryptoTypes.SharedKey256 sharedKey = SharedKey.deriveSharedKeyDeprecated(this.keyPair, recipientPublicKey, salt);
		final CipherHelpers.EncodedCbc encoded = CipherHelpers.encodeAesCbc(sharedKey, message);

		final Message encodedMessage = new Message();
		encodedMessage.setMessageType(MessageType.ENCRYPTED);
		encodedMessage.setMessage(ArrayHelpers.concat(salt, encoded.initializationVector(), encoded.cipherText()));
		return encodedMessage;
	}

	// region cipher helpers

	private byte[] tryDecodeGcm(final CryptoTypes.PublicKey otherPublicKey, final byte[] payload) {
		// derive the shared key before the helper's length guard, so an off-curve key throws
		// "invalid point" even for a short payload rather than being silently reported as not-decoded
		final CryptoTypes.SharedKey256 sharedKey = SharedKey.deriveSharedKey(this.keyPair, otherPublicKey);
		return CipherHelpers.tryDecodeAesGcm(sharedKey, payload);
	}

	private byte[] tryDecodeCbc(final CryptoTypes.PublicKey otherPublicKey, final byte[] payload) {
		if (payload.length < SALT_SIZE)
			throw new IllegalArgumentException("invalid salt");

		// the salt participates in the deprecated key derivation, so it is split off here rather than in the helper
		final byte[] salt = Arrays.copyOfRange(payload, 0, SALT_SIZE);
		final CryptoTypes.SharedKey256 sharedKey = SharedKey.deriveSharedKeyDeprecated(this.keyPair, otherPublicKey, salt);
		return CipherHelpers.tryDecodeAesCbc(sharedKey, Arrays.copyOfRange(payload, SALT_SIZE, payload.length));
	}

	// endregion
}
